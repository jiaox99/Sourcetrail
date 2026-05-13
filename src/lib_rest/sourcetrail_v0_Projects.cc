#include "sourcetrail_v0_Projects.h"
#include "service/ProjectService.h"
#include "rest_util.h"

using namespace sourcetrail::v0;

void Projects::list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
	// Delegate to service layer
	Json::Value result = sourcetrail::service::listProjects();
	auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
	callback(resp);
}

void Projects::load(const ProjectLoadRequest&& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	// Use service layer
	FilePath projectPath(req.projectFilePath);
	auto result = sourcetrail::service::loadProject(projectPath);

	message_response(result.message, callback);
}