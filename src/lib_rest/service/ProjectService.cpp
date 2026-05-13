#include "ProjectService.h"
#include "Application.h"
#include "ApplicationSettings.h"
#include "MessageLoadProject.h"

namespace sourcetrail {
namespace service {

Json::Value listProjects() {
	Json::Value projects(Json::arrayValue);

	auto recentProjects = ApplicationSettings::getInstance()->getRecentProjects();
	for (const auto& project : recentProjects) {
		if (project.exists()) {
			projects.append(project.str());
		}
	}

	Json::Value result;
	result["projects"] = projects;

	FilePath currentProjectPath = Application::getInstance()->getCurrentProjectPath();
	if (!currentProjectPath.empty()) {
		result["currentProject"] = currentProjectPath.str();
	} else {
		result["currentProject"] = Json::Value(Json::nullValue);
	}

	return result;
}

LoadProjectResult loadProject(const FilePath& projectPath) {
	if (!projectPath.exists()) {
		return {false, "Project path does not exist"};
	}

	MessageLoadProject(projectPath).dispatch();
	return {true, "Project loading started"};
}

}} // namespace
