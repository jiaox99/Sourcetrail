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
#include "flashmapper.h"

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

class SearchIndex : flashmapper::ComplexMapper
{
public:
	SearchIndex();
	virtual ~SearchIndex();

	flashmapper::Address writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block);
	void resolveData(flashmapper::DataBlock& block);

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

	public:
		flashmapper::map<Id, NodeType> elementIds;
		NodeTypeSet containedTypes;
		flashmapper::map<wchar_t, long> edges;
	};

	struct SearchEdge
	{
		SearchEdge(): SearchEdge(0, L"") {}
		SearchEdge(long target, std::wstring s): target(target), s(s.c_str()) {}

	public:
		long target;
		flashmapper::wstring s;
		flashmapper::set<wchar_t> gate;
	};

	struct SearchPath
	{
		SearchPath(std::wstring text, std::vector<size_t> indices, SearchNode* node)
			: text(text.c_str()), indices(std::move(indices)), node(node)
		{
		}

		flashmapper::wstring text;
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

	flashmapper::vector<SearchNode> m_nodes;
	flashmapper::vector<SearchEdge> m_edges;
};

#endif	  // SEARCH_INDEX_H
