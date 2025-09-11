#include "sourcetrail_v0_Symbols.h"
#include "rest_api_main.h"
#include "StorageAccess.h"
#include "boost/locale.hpp"
#include "NodeTypeSet.h"
#include "Graph.h"
#include "NameHierarchy.h"

using namespace sourcetrail::v0;
using namespace drogon;

std::wstring to_wstring(const std::string& str)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(str);
}

std::string to_string(const std::wstring& wstr)
{
	return boost::locale::conv::utf_to_utf<char>(wstr);
}

// Add definition of your processing function here
void Symbols::fuzzyQuery(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& query) const
{
	int limit = req->getOptionalParameter<int>("max").value_or(50);
	auto *storage = getStorageInstance();
	auto result = storage->getAutocompletionMatches(to_wstring(query), NodeTypeSet::all(), false);
	Json::Value jsonResult;
	int count = 0;
	for (const auto& entry: result)
	{
		for (const auto& tokenName : entry.tokenNames)
		{
			jsonResult.append(to_string(NameHierarchy::serialize(tokenName)));
			if (++count >= limit)
				break;
		}
	}
	auto resp = HttpResponse::newHttpJsonResponse(jsonResult);
	callback(resp);
}

void Symbols::graphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	auto *storage = getStorageInstance();
	
	std::vector<Id> symbolIds;
	for (const auto& symbolFullName: query.symbolFullNames)
	{
		NameHierarchy symbolHierarchy = NameHierarchy::deserialize(to_wstring(symbolFullName));
		auto symbolId = storage->getNodeIdForNameHierarchy(symbolHierarchy);
		if (symbolId != 0)
			symbolIds.push_back(symbolId);
	}
	std::vector<Id> expandedNodeIds;
	bool isNameSpace = false;
	auto graph = storage->getGraphForActiveTokenIds(symbolIds, expandedNodeIds, &isNameSpace);

	auto resp = HttpResponse::newHttpResponse();
	resp->setContentTypeCode(CT_TEXT_PLAIN);
	resp->setStatusCode(k200OK);
	std::wstringstream wss;
	graph->print(wss);
	resp->setBody(to_string(wss.str()));
	callback(resp);
}