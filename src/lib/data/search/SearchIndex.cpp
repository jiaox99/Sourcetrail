#include "SearchIndex.h"

#include <algorithm>
#include <ctype.h>
#include <iterator>

#include "utility.h"
#include "utilityString.h"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/serialization/unique_ptr.hpp"
#include <iostream>
#include <fstream>

#pragma optimize("", off)

SearchIndex::SearchIndex()
{
	clear();
}

SearchIndex::~SearchIndex() {}

void SearchIndex::addNode(Id id, std::wstring name, NodeType type)
{
	size_t currentNodeI = 0;	   // SearchNode* currentNode = &m_nodes[0];

	while (name.size() > 0)
	{
		SearchNode* currentNode = &m_nodes[currentNodeI];
		currentNode->containedTypes.add(type);

		auto it = currentNode->edges.find(name[0]);
		if (it != currentNode->edges.end())
		{
			size_t currentEdgeI = *it->second;
			SearchEdge* currentEdge = &m_edges[currentEdgeI];
			const flashmapper::wstring& edgeString = currentEdge->s;

			size_t matchCount = 1;
			for (size_t j = 1; j < edgeString.size() && j < name.size(); j++)
			{
				if (edgeString[j] != name[j])
				{
					break;
				}
				matchCount++;
			}

			if (matchCount < edgeString.size())
			{
				// split current edge
				m_nodes.push_back(SearchNode(currentNode->containedTypes));
				SearchNode* n = m_nodes.back();

				m_edges.push_back(SearchEdge(
					currentEdge->target, edgeString.substr(matchCount).data()));
				SearchEdge* e = m_edges.back();

				n->edges.emplace(e->s[0], m_edges.size()-1);

				currentEdge = &m_edges[currentEdgeI];
				currentEdge->s = m_edges[currentEdgeI].s.substr(0, matchCount);
				currentEdge->target = (long)m_nodes.size() - 1;
			}

			name = name.substr(matchCount);
			currentNodeI = currentEdge->target;
		}
		else
		{
			m_nodes.push_back(SearchNode(currentNode->containedTypes));
			SearchNode* n = m_nodes.back();

			m_edges.push_back(SearchEdge((long)m_nodes.size() - 1, std::move(name)));
			SearchEdge* e = m_edges.back();

			m_nodes[currentNodeI].edges.emplace(e->s[0], m_edges.size() - 1);
			currentNodeI = m_nodes.size() - 1;

			name.clear();
		}
	}

	m_nodes[currentNodeI].elementIds.emplace(id, type);
}

void SearchIndex::finishSetup()
{
	for (auto& p: m_nodes[0].edges)
	{
		populateEdgeGate(&m_edges[*p.second]);
	}
}

void SearchIndex::clear()
{
	m_nodes.clear();
	m_edges.clear();

	m_nodes.push_back(SearchNode());
}

std::vector<SearchResult> SearchIndex::search(
	const std::wstring& query,
	NodeTypeSet acceptedNodeTypes,
	size_t maxResultCount,
	size_t maxBestScoredResultsLength) const
{
	// find paths containing query
	std::vector<SearchPath> paths;
	searchRecursive(
		SearchPath(L"", {}, &m_nodes[0]), utility::toLowerCase(query), acceptedNodeTypes, &paths);

	// create scored search results
	std::multiset<SearchResult> searchResults = createScoredResults(
		paths, acceptedNodeTypes, maxResultCount * 3);

	// find maximum length for best scores
	std::multiset<size_t> resultLengths;
	for (const SearchResult& result: searchResults)
	{
		resultLengths.insert(result.text.size());
	}
	size_t maxResultLength = 0;
	if (resultLengths.size() > 1000)
	{
		auto it = resultLengths.begin();
		std::advance(it, 1000);
		maxResultLength = *it;
	}

	// find best scores
	std::map<std::wstring, SearchResult> scoresCache;
	std::multiset<SearchResult> bestResults;
	for (const SearchResult& result: searchResults)
	{
		if (!maxResultLength || result.text.size() <= maxResultLength)
		{
			bestResults.insert(bestScoredResult(result, &scoresCache, maxBestScoredResultsLength));
		}
	}

	// narrow down to max result count
	auto it = bestResults.end();
	if (maxResultCount && bestResults.size() > maxResultCount)
	{
		it = bestResults.begin();
		std::advance(it, maxResultCount);
	}

	return std::vector<SearchResult>(bestResults.begin(), it);
}

void SearchIndex::populateEdgeGate(SearchEdge* e)
{
	for (auto& p: m_nodes[e->target].edges)
	{
		SearchEdge* targetEdge = &m_edges[*p.second];
		populateEdgeGate(targetEdge);
#if 0
		std::cout << "PreGate:" << std::endl;
		for (auto c : e->gate)
		{
			std::wcout << c << std::endl;
		}

		std::cout << "MergingGate:" << std::endl;
		for (auto c: targetEdge->gate)
		{
			std::wcout << c << std::endl;
		}
#endif // #if 0
		utility::append(e->gate, targetEdge->gate);
	}

	for (const wchar_t& c: e->s)
	{
		e->gate.insert(towlower(c));
	}
}

void SearchIndex::searchRecursive(
	const SearchPath& path,
	const std::wstring& remainingQuery,
	NodeTypeSet acceptedNodeTypes,
	std::vector<SearchIndex::SearchPath>* results) const
{
	for (const auto& p: path.node->edges)
	{
		const SearchEdge* currentEdge = &m_edges[*p.second];

		if (!acceptedNodeTypes.intersectsWith(m_nodes[currentEdge->target].containedTypes))
		{
			continue;
		}

		// test if s passes the edge's gate.
		bool passesGate = true;
		for (const wchar_t& c: remainingQuery)
		{
			if (currentEdge->gate.find(c) == currentEdge->gate.end())
			{
				passesGate = false;
				break;
			}
		}

		if (!passesGate)
		{
			continue;
		}

		// consume characters for edge
		const flashmapper::wstring& edgeString = currentEdge->s;
		SearchPath currentPath {(path.text + edgeString).data(), path.indices, &m_nodes[currentEdge->target]};

		size_t j = 0;
		for (size_t i = 0; i < edgeString.size() && j < remainingQuery.size(); i++)
		{
			if (towlower(edgeString[i]) == remainingQuery[j])
			{
				currentPath.indices.push_back(path.text.size() + i);
				j++;
			}
		}

		if (j == remainingQuery.size())
		{
			results->push_back(std::move(currentPath));
		}
		else
		{
			searchRecursive(currentPath, remainingQuery.substr(j), acceptedNodeTypes, results);
		}
	}
}

std::multiset<SearchResult> SearchIndex::createScoredResults(
	const std::vector<SearchPath>& paths, NodeTypeSet acceptedNodeTypes, size_t maxResultCount) const
{
	// score and order initial paths
	std::multimap<int, SearchPath, std::greater<int>> scoredPaths;
	for (const SearchPath& path: paths)
	{
		scoredPaths.emplace(scoreText(path.text.data(), path.indices), path);
	}

	// score paths and subpaths
	std::multiset<SearchResult> searchResults;
	for (const std::pair<int, SearchPath>& p: scoredPaths)
	{
		std::vector<SearchPath> currentPaths;
		currentPaths.push_back(p.second);

		while (!currentPaths.empty())
		{
			std::vector<SearchPath> nextPaths;

			for (const SearchPath& path: currentPaths)
			{
				if (!path.node->elementIds.empty() &&
					(acceptedNodeTypes.intersectsWith(path.node->containedTypes)))
				{
					std::vector<Id> elementIds;
					for (const auto& p: path.node->elementIds)
					{
						if (acceptedNodeTypes.contains(*p.second))
						{
							elementIds.push_back(p.first);
						}
					}

					if (!elementIds.empty())
					{
						searchResults.emplace(
							path.text.data(),
							std::move(elementIds),
							path.indices,
							scoreText(path.text.data(), path.indices));

						if (maxResultCount && searchResults.size() >= maxResultCount)
						{
							return searchResults;
						}
					}
				}

				for (auto p: path.node->edges)
				{
					const SearchEdge* edge = &m_edges[*p.second];
					nextPaths.emplace_back((path.text + edge->s).data(), path.indices, &m_nodes[edge->target]);
				}
			}

			currentPaths = std::move(nextPaths);
		}
	}

	return searchResults;
}

SearchResult SearchIndex::bestScoredResult(
	SearchResult result,
	std::map<std::wstring, SearchResult>* scoresCache,
	size_t maxBestScoredResultsLength)
{
	const std::wstring text = result.text;

	if (maxBestScoredResultsLength && result.text.size() > maxBestScoredResultsLength)
	{
		if (result.indices.back() >= maxBestScoredResultsLength)
		{
			return result;
		}

		result.text = result.text.substr(0, maxBestScoredResultsLength);
	}

	auto it = scoresCache->find(result.text);
	if (it != scoresCache->end())
	{
		// std::cout << "cached: " << it->first << " " << it->second.score << std::endl;
		SearchResult cachedResult = it->second;
		cachedResult.text = text;
		cachedResult.elementIds = result.elementIds;
		return cachedResult;
	}

	const std::vector<size_t> indices = result.indices;
	bestScoredResultRecursive(
		utility::toLowerCase(result.text),
		indices,
		indices.back(),
		indices.size() - 1,
		scoresCache,
		&result);

	// std::cout << "save: " << result.text << " " << result.score << std::endl;
	scoresCache->emplace(result.text, result);

	result.text = text;

	return result;
}

void SearchIndex::bestScoredResultRecursive(
	const std::wstring& lowerText,
	const std::vector<size_t>& indices,
	const size_t lastIndex,
	const size_t indicesPos,
	std::map<std::wstring, SearchResult>* scoresCache,
	SearchResult* result)
{
	// left for debugging
	// std::cout << lowerText << std::endl;
	// size_t idx = 0;
	// for (size_t i = 0; i < lowerText.size() && idx < indices.size(); i++)
	// {
	// 	if (i == indices[idx])
	// 	{
	// 		if (idx == indicesPos)
	// 			std::cout << "I";
	// 		else
	// 			std::cout << "^";
	// 		idx++;
	// 	}
	// 	else
	// 		std::cout << " ";
	// }
	// std::cout << "\n" << std::endl;

	if (indicesPos + 1 == indices.size())
	{
		for (size_t i = (indices.back() == lastIndex ? lowerText.size() - 1 : indices.back() - 1);
			 i > lastIndex;
			 i--)
		{
			if (lowerText[i] == lowerText[lastIndex])
			{
				std::wstring lowerTextPart = result->text.substr(0, i + 1);

				auto it = scoresCache->find(lowerTextPart);
				if (it != scoresCache->end())
				{
					// std::cout << "cached: " << it->first << " " << it->second.score << std::endl;
					result->score = it->second.score;
					result->indices = it->second.indices;
					return;
				}

				std::vector<size_t> newIndices = indices;
				newIndices[indicesPos] = i;

				int newScore = scoreText(result->text, newIndices);
				if (newScore > result->score)
				{
					result->score = newScore;
					result->indices = newIndices;
				}

				bestScoredResultRecursive(
					lowerText, newIndices, lastIndex, indicesPos, scoresCache, result);

				// std::cout << "save: " << lowerTextPart << " " << result->score << std::endl;
				scoresCache->emplace(lowerTextPart, *result);
				break;
			}
		}
	}
	else
	{
		size_t oldTextPos = indices[indicesPos];
		size_t nextTextPos = indices[indicesPos + 1];

		for (size_t i = oldTextPos + 1; i < nextTextPos; i++)
		{
			if (lowerText[i] == lowerText[oldTextPos])
			{
				std::vector<size_t> newIndices = indices;
				newIndices[indicesPos] = i;

				int newScore = scoreText(result->text, newIndices);
				if (newScore > result->score)
				{
					result->score = newScore;
					result->indices = newIndices;
				}

				bestScoredResultRecursive(
					lowerText, newIndices, lastIndex, indicesPos, scoresCache, result);
				break;
			}
		}
	}

	for (size_t i = indicesPos; i > 0; i--)
	{
		if (indices[i] - indices[i - 1] > 1)
		{
			bestScoredResultRecursive(lowerText, indices, lastIndex, i - 1, scoresCache, result);
			break;
		}
	}
}

int SearchIndex::scoreText(const std::wstring& text, const std::vector<size_t>& indices)
{
	const int unmatchedLetterBonus = -1;
	const int consecutiveLetterBonus = 4;
	const int camelCaseBonus = 3;
	const int noLetterBonus = 4;
	const int firstLetterBonus = 4;
	const int delayedStartBonus = -1;
	const int minDelayedStartBonus = -20;

	int unmatchedLetterScore = 0;
	int consecutiveLetterScore = 0;
	int camelCaseScore = 0;
	int noLetterScore = 0;
	int firstLetterScore = 0;

	for (int i = 0; i < static_cast<int>(indices.size()); i++)
	{
		// unmatched and consecutive
		if (i > 0)
		{
			unmatchedLetterScore += static_cast<int>(
				(indices[i] - indices[i - 1] - 1) * unmatchedLetterBonus);
			consecutiveLetterScore += (indices[i] - indices[i - 1] == 1) ? consecutiveLetterBonus : 0;
		}

		size_t index = indices[i];

		// first letter
		if (index == 0)
		{
			firstLetterScore += firstLetterBonus;
		}
		// after no letter
		else if (index != 0 && isNoLetter(text[index - 1]))
		{
			noLetterScore += noLetterBonus;
		}
		// camel case
		else if (iswupper(text[index]))
		{
			bool prevIsLower = (index > 0 && iswlower(text[index - 1]));
			bool nextIsLower = (index + 1 < text.size() && iswlower(text[index + 1]));

			if (prevIsLower || nextIsLower)
			{
				camelCaseScore += camelCaseBonus;
			}
		}
	}

	int leadingStartScore = std::max(int(indices[0]) * delayedStartBonus, minDelayedStartBonus);

	int score = unmatchedLetterScore + consecutiveLetterScore + camelCaseScore + noLetterScore +
		firstLetterScore + leadingStartScore;

	// left for debugging
	// std::cout << unmatchedLetterScore << " " << consecutiveLetterScore << " " << camelCaseScore
	// << " " << noLetterScore; std::cout << " " << firstLetterScore << " " << leadingStartScore <<
	// " - " << score << " " << text << std::endl;

	return score;
}

SearchResult SearchIndex::rescoreText(
	const std::wstring& fulltext,
	const std::wstring& text,
	const std::vector<size_t>& indices,
	int score,
	size_t maxBestScoredResultsLength)
{
	SearchResult result(text, {}, indices, score);
	std::vector<size_t> textIndices;

	// match is already within text
	const int newIdx = static_cast<int>(indices[0] - (fulltext.size() - text.size()));
	if (newIdx >= 0)
	{
		for (size_t idx: indices)
		{
			textIndices.push_back(idx - (fulltext.size() - text.size()));
		}
	}
	// try if match is within text
	else
	{
		size_t idx = 0;
		size_t textSize = text.size();
		if (maxBestScoredResultsLength && textSize > maxBestScoredResultsLength)
		{
			textSize = maxBestScoredResultsLength;
		}

		for (size_t i = 0; i < textSize && idx < indices.size(); i++)
		{
			if (towlower(text[i]) == towlower(fulltext[indices[idx]]))
			{
				textIndices.push_back(i);
				idx++;
			}
		}

		// match was not found
		if (idx != indices.size())
		{
			result.score -= 1;
			return result;
		}
	}

	result.score = scoreText(text, textIndices);
	result.indices = textIndices;

	std::map<std::wstring, SearchResult> scoresCache;
	result = bestScoredResult(result, &scoresCache, maxBestScoredResultsLength);

	for (size_t i = 0; i < result.indices.size(); i++)
	{
		result.indices[i] += fulltext.size() - text.size();
	}

	return result;
}

bool SearchIndex::isNoLetter(const wchar_t c)
{
	switch (c)
	{
	case L' ':
	case L'.':
	case L',':
	case L'_':
	case L':':
	case L'<':
	case L'>':
	case L'/':
	case L'\\':
		return true;
	}
	return false;
}

flashmapper::Address SearchIndex::writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block)
{
	mapper.writeData(m_nodes, block);
	mapper.writeData(m_edges, block);

	return block.baseOffset + block.cursor;

}

void SearchIndex::resolveData(flashmapper::DataBlock& block)
{
	m_nodes.resolveData(block);
	m_edges.resolveData(block);
}

flashmapper::Address SearchIndex::SearchNode::writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block)
{
	mapper.writeData(elementIds, block);
	mapper.writeData(edges, block);
	mapper.writeData(containedTypes, block);

	return block.baseOffset + block.cursor;
}

void SearchIndex::SearchNode::resolveData(flashmapper::DataBlock& block)
{
	elementIds.resolveData(block);
	edges.resolveData(block);
}

flashmapper::Address SearchIndex::SearchEdge::writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block)
{
	mapper.writeData(s, block);
	mapper.writeData(gate, block);
	mapper.writeData(target, block);

	return block.baseOffset + block.cursor;
}

void SearchIndex::SearchEdge::resolveData(flashmapper::DataBlock& block)
{
	s.resolveData(block);
	gate.resolveData(block);
}

void SearchIndex::load(std::string filePath, flashmapper::Mapper& mapper)
{
	m_nodes.clear();
	m_edges.clear();

	mapper.readFromFile(filePath.c_str());
	SearchIndex* index = mapper.readData<SearchIndex>();
	m_nodes = std::move(index->m_nodes);
	m_edges = std::move(index->m_edges);
}

void SearchIndex::save(std::string filePath, flashmapper::Mapper& mapper)
{
	mapper.reset();
	flashmapper::DataBlock block = mapper.requestBlock(sizeof(SearchIndex));
	mapper.writeData(*this, block);
	block.align();
	assert(block.postValidate());
	mapper.writeToFile(filePath.c_str());
}