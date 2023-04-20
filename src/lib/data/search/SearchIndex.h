#ifndef SEARCH_INDEX_H
#define SEARCH_INDEX_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Node.h"
#include "NodeTypeSet.h"
#include "types.h"
#include "boost/serialization/set.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/access.hpp"

// SearchResult is only used as an internal type in the SearchIndex and the PersistentStorage
struct SearchResult
{
	SearchResult(std::wstring text, std::vector<Id> elementIds, std::vector<size_t> indices, int score)
		: text(std::move(text))
		, elementIds(std::move(elementIds))
		, indices(std::move(indices))
		, score(score)
	{
	}

	bool operator<(const SearchResult& other) const
	{
		return score > other.score;
	}

	std::wstring text;
	std::vector<Id> elementIds;
	std::vector<size_t> indices;
	int score;
};

class SearchIndex
{
public:
	SearchIndex();
	virtual ~SearchIndex();

	void addNode(Id id, std::wstring name, NodeType type = NodeType(NODE_SYMBOL));
	void finishSetup();
	void clear();

	void load(std::string filePath);
	void save(std::string filePath);

	// maxResultCount == 0 means "no restriction".
	std::vector<SearchResult> search(
		const std::wstring& query,
		NodeTypeSet acceptedNodeTypes,
		size_t maxResultCount,
		size_t maxBestScoredResultsLength = 0) const;

private:
	struct SearchEdge;

	struct SearchNode
	{
		SearchNode(): SearchNode(NodeTypeSet()) {}

		SearchNode(NodeTypeSet containedTypes): containedTypes(containedTypes) {}

	private:
		friend class boost::serialization::access;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			(void)version;
			ar & elementIds;
			ar & containedTypes;
			ar & edges;
		}

	public:
		std::map<Id, NodeType> elementIds;
		NodeTypeSet containedTypes;
		std::map<wchar_t, long> edges;
	};

	struct SearchEdge
	{
		SearchEdge(): SearchEdge(0, L"") {}
		SearchEdge(long target, std::wstring s): target(target), s(std::move(s)) {}

	private:
		friend class boost::serialization::access;

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			(void)version;
			ar & target;
			ar & s;
			ar & gate;
		}

	public:
		long target;
		std::wstring s;
		std::set<wchar_t> gate;
	};

	struct SearchPath
	{
		SearchPath(std::wstring text, std::vector<size_t> indices, SearchNode* node)
			: text(std::move(text)), indices(std::move(indices)), node(node)
		{
		}

		std::wstring text;
		std::vector<size_t> indices;
		SearchNode* node;
	};

	void populateEdgeGate(SearchEdge* e);
	void searchRecursive(
		const SearchPath& path,
		const std::wstring& remainingQuery,
		NodeTypeSet acceptedNodeTypes,
		std::vector<SearchIndex::SearchPath>* results) const;

	std::multiset<SearchResult> createScoredResults(
		const std::vector<SearchPath>& paths,
		NodeTypeSet acceptedNodeTypes,
		size_t maxResultCount) const;

	static SearchResult bestScoredResult(
		SearchResult result,
		std::map<std::wstring, SearchResult>* scoresCache,
		size_t maxBestScoredResultsLength);
	static void bestScoredResultRecursive(
		const std::wstring& lowerText,
		const std::vector<size_t>& indices,
		const size_t lastIndex,
		const size_t indicesPos,
		std::map<std::wstring, SearchResult>* scoresCache,
		SearchResult* result);
	static int scoreText(const std::wstring& text, const std::vector<size_t>& indices);

public:
	static SearchResult rescoreText(
		const std::wstring& fulltext,
		const std::wstring& text,
		const std::vector<size_t>& indices,
		int score,
		size_t maxBestScoredResultsLength);

	static bool isNoLetter(const wchar_t c);

	std::vector<std::unique_ptr<SearchNode>> m_nodes;
	std::vector<std::unique_ptr<SearchEdge>> m_edges;
	SearchNode* m_root;
};

#endif	  // SEARCH_INDEX_H
