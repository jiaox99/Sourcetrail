#pragma once

#include <drogon/HttpController.h>

struct ProjectLoadRequest
{
	std::string projectFilePath;
};

namespace drogon
{

template <>
inline ProjectLoadRequest fromRequest(const drogon::HttpRequest& req)
{
	auto json = req.getJsonObject();
	ProjectLoadRequest loadReq;
	if (json == nullptr)
	{
		return loadReq;
	}
	if (json->isMember("projectFilePath"))
	{
		loadReq.projectFilePath = (*json)["projectFilePath"].asString();
	}
	return loadReq;
}

} // namespace drogon

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
    void load(const ProjectLoadRequest&& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) const;
};
}
}
