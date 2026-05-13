#pragma once
#include <json/json.h>
#include <string>
#include <vector>
#include "types.h"

class StorageAccess;

namespace sourcetrail {
namespace service {

struct FuzzySearchParams {
	std::wstring query;
	int maxResults = 50;
};

// Fuzzy search for symbols
Json::Value fuzzySearchSymbols(const FuzzySearchParams& params, StorageAccess* storage);

struct GraphQueryParams {
	std::vector<std::string> symbolFullNames;
	int maxDepth = 5;
	std::vector<std::string> nodeTypes;
	std::vector<std::string> edgeTypes;
};

// Generate graph visualization (PlantUML format)
std::wstring generateSymbolGraph(const GraphQueryParams& params, StorageAccess* storage);

// Custom trail query between two symbols
std::wstring generateCustomGraph(const GraphQueryParams& params, StorageAccess* storage);

}} // namespace
