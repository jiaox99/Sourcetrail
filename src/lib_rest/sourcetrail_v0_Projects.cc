#include "sourcetrail_v0_Projects.h"
#include "ApplicationSettings.h"
#include "MessageLoadProject.h"
#include "rest_util.h"

using namespace sourcetrail::v0;

static Json::Value s_jsonProjects;

void Projects::list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
	if (s_jsonProjects.empty())
	{
		auto projects = ApplicationSettings::getInstance()->getRecentProjects();
		Json::Value jsonResult;
		for (const auto& project: projects)
		{
			if (project.exists())
			{
				jsonResult.append(project.str());
			}
		}
	}

	if (!s_jsonProjects.empty())
	{
		auto resp = drogon::HttpResponse::newHttpJsonResponse(s_jsonProjects);
		callback(resp);
	}
	else
	{
		message_response("No projects found", callback);
	}
}

void Projects::load(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& query) const
{
	FilePath projectFilePath(query);
	if (!projectFilePath.exists())
	{
		message_response("Project path is not exist", callback);
		return;
	}

	MessageLoadProject(projectFilePath).dispatch();

	message_response("Start loading project", callback);
}