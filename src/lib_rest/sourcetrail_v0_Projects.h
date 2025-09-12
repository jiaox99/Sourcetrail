#pragma once

#include <drogon/HttpController.h>

namespace sourcetrail
{
namespace v0
{
class Projects : public drogon::HttpController<Projects>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Projects::list, "/list", drogon::Get);
	METHOD_ADD(Projects::load, "/load/{}", drogon::Post);

    METHOD_LIST_END
private:
	void list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void load(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback, const std::string& query) const;
};
}
}
