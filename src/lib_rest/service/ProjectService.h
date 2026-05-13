#pragma once
#include <json/json.h>
#include "FilePath.h"

namespace sourcetrail {
namespace service {

// List recent projects
Json::Value listProjects();

// Load a project by path
struct LoadProjectResult {
	bool success;
	std::string message;
};
LoadProjectResult loadProject(const FilePath& projectPath);

}} // namespace sourcetrail::service
