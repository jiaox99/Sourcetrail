#pragma once

#include <drogon/HttpController.h>

struct SymbolQueryRequest
{
	std::vector<std::string> symbolFullNames;
};

namespace drogon
{

template <>
inline SymbolQueryRequest fromRequest(const HttpRequest& req)
{
	auto json = req.getJsonObject();
	SymbolQueryRequest queryReq;
	for (auto& symbolFullName : (*json)["symbolFullNames"])
	{
		queryReq.symbolFullNames.push_back(symbolFullName.asString());
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
    METHOD_LIST_END
private:
	void fuzzyQuery(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& query) const;
	void graphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
};
}
}
