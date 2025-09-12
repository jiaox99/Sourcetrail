#pragma once

#include <drogon/HttpController.h>

struct SymbolQueryRequest
{
	std::vector<std::string> symbolFullNames;
	int maxDepth = 5;
	std::vector<std::string> nodeTypes;
	std::vector<std::string> edgeTypes;
};

namespace drogon
{

template <>
inline SymbolQueryRequest fromRequest(const HttpRequest& req)
{
	auto json = req.getJsonObject();
	SymbolQueryRequest queryReq;
	if (json == nullptr)
	{
		return queryReq;
	}

	if (!json->find("symbolFullNames")->isNull())
	{
		for (auto& symbolFullName: (*json)["symbolFullNames"])
		{
			queryReq.symbolFullNames.push_back(symbolFullName.asString());
		}
	}

	if (!json->find("nodeTypes")->isNull())
	{
		for (auto& nodeType: (*json)["nodeTypes"])
		{
			queryReq.nodeTypes.push_back(nodeType.asString());
		}
	}

	if (!json->find("edgeTypes")->isNull())
	{
		for (auto& edgeType: (*json)["edgeTypes"])
		{
			queryReq.edgeTypes.push_back(edgeType.asString());
		}
	}

	if (!json->find("maxDepth")->isNull())
	{
		queryReq.maxDepth = (*json)["maxDepth"].asInt();
	}

	return queryReq;
}

}

namespace sourcetrail
{
namespace v0
{

class Symbols : public drogon::HttpController<Symbols>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Symbols::fuzzyQuery, "/fuzzyQuery/{}", drogon::Get);
	METHOD_ADD(Symbols::graphQuery, "/graphQuery", drogon::Post);
	METHOD_ADD(Symbols::customGraphQuery, "/customGraphQuery", drogon::Post);
    METHOD_LIST_END
private:
	void fuzzyQuery(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& query) const;
	void graphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
	void customGraphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
};
}
}
