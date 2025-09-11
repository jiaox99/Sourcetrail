#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace sourcetrail
{
namespace v0
{
class Symbols : public drogon::HttpController<Symbols>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Symbols::fuzzyQuery, "/fuzzyQuery/{}", Get);
    METHOD_LIST_END
private:
	void fuzzyQuery(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& query) const;
};
}
}
