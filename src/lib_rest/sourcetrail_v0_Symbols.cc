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
			Json::Value tokenValue;
			tokenValue["fullName"] = to_string(tokenName.getQualifiedName());
			tokenValue["serializedName"] = to_string(NameHierarchy::serialize(tokenName));
			jsonResult.append(tokenValue);
			if (++count >= limit)
				break;
		}

		if (count >= limit)
			break;
	}
	auto resp = HttpResponse::newHttpJsonResponse(jsonResult);
	callback(resp);
}

void errorResponse(const std::string& message, std::function<void(const drogon::HttpResponsePtr&)>& callback)
{
	auto resp = HttpResponse::newHttpResponse();
	resp->setContentTypeCode(CT_TEXT_PLAIN);
	resp->setStatusCode(k200OK);
	resp->setBody(message);
	callback(resp);
}

bool validateQueryRequest(const SymbolQueryRequest& query, std::function<void(const drogon::HttpResponsePtr&)>& callback)
{
	if (query.symbolFullNames.empty())
	{
		errorResponse("No symbolFullNames provided", callback);
		return false;
	}

	return true;
}

void symbolNameToIds(const std::vector<std::string>& symbolFullNames, std::vector<Id>& outSymbolIds, StorageAccess* storage)
{
	for (const auto& symbolFullName: symbolFullNames)
	{
		NameHierarchy symbolHierarchy = NameHierarchy::deserialize(to_wstring(symbolFullName));
		auto symbolId = storage->getNodeIdForNameHierarchy(symbolHierarchy);
		if (symbolId != 0)
			outSymbolIds.push_back(symbolId);
	}
}

void Symbols::graphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	if (!validateQueryRequest(query, callback))
	{
		return;
	}

	auto *storage = getStorageInstance();
	std::vector<Id> symbolIds;
	symbolNameToIds(query.symbolFullNames, symbolIds, storage);
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

NodeKindMask fromNodeKindStrings(const std::vector<std::string>& nodeKinds)
{
	NodeKindMask mask = 0;
	for (const auto& nodeKindStr: nodeKinds)
	{
		mask |= getNodeKindForReadableNodeKindString(to_wstring(nodeKindStr));
	}
	mask = mask == 0 ? ~0 : mask;
	return mask;
}

Edge::TypeMask fromEdgeTypeStrings(const std::vector<std::string>& edgeTypes)
{
	Edge::TypeMask mask = 0;
	for (const auto& edgeTypeStr: edgeTypes)
	{
		mask |= Edge::getTypeForReadableTypeString(to_wstring(edgeTypeStr));
	}

	mask = mask == 0 ? ~0 : mask;
	return mask;
}

void Symbols::customGraphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	if (!validateQueryRequest(query, callback))
	{
		return;
	}

	auto* storage = getStorageInstance();
	std::vector<Id> symbolIds;
	symbolNameToIds(query.symbolFullNames, symbolIds, storage);
	if (symbolIds.size() != 2)
	{
		errorResponse("Two Symbol Names needed", callback);
		return;
	}
	auto graph = storage->getGraphForTrail(symbolIds[0], symbolIds[1], fromNodeKindStrings(query.nodeTypes), fromEdgeTypeStrings(query.edgeTypes), true, query.maxDepth, true);
}