#include "MCPToolRegistry.h"
#include "service/ProjectService.h"
#include "service/SymbolService.h"
#include "service/CodeService.h"
#include "rest_util.h"
#include "FilePath.h"
#include "ResourcePaths.h"
#include "logging.h"
#include <fstream>

namespace sourcetrail {
namespace mcp {

MCPToolRegistry::MCPToolRegistry() {
	// Try to load from JSON first
	FilePath jsonPath = ResourcePaths::getMCPDirectoryPath().concatenate(L"tools.json");
	if (jsonPath.exists()) {
		registerToolsFromJson(jsonPath.str());
	} else {
		// Fallback to hardcoded registration
		LOG_WARNING("MCP tools config not found at: " + jsonPath.str() + ", using hardcoded definitions");
	}
}

void MCPToolRegistry::registerToolsFromJson(const std::string& jsonPath) {
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		LOG_ERROR("Failed to open MCP tools config: " + jsonPath);
		return;
	}

	Json::Value root;
	Json::CharReaderBuilder builder;
	std::string errs;
	if (!Json::parseFromStream(builder, file, &root, &errs)) {
		LOG_ERROR("Failed to parse MCP tools JSON: " + errs);
		return;
	}

	const Json::Value& tools = root["tools"];
	if (!tools.isArray()) {
		LOG_ERROR("MCP tools JSON: 'tools' field must be an array");
		return;
	}

	for (const auto& toolJson : tools) {
		ToolDefinition def;
		def.name = toolJson["name"].asString();
		def.description = toolJson["description"].asString();
		def.inputSchema = toolJson["inputSchema"];

		std::string handlerName = toolJson["handlerName"].asString();
		ToolHandler handler = createHandler(handlerName);

		if (handler) {
			registerTool(def, handler);
			LOG_INFO("Registered MCP tool: " + def.name);
		} else {
			LOG_ERROR("Unknown handler name: " + handlerName + " for tool: " + def.name);
		}
	}
}

ToolHandler MCPToolRegistry::createHandler(const std::string& handlerName) {
	if (handlerName == "list_projects") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			return service::listProjects();
		};
	}
	else if (handlerName == "load_project") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			std::string pathStr = args["projectFilePath"].asString();
			FilePath path(pathStr);
			auto result = service::loadProject(path);

			Json::Value response;
			response["success"] = result.success;
			response["message"] = result.message;
			return response;
		};
	}
	else if (handlerName == "fuzzy_search") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			service::FuzzySearchParams params;
			params.query = to_wstring(args["query"].asString());
			params.maxResults = args.get("maxResults", 50).asInt();

			return service::fuzzySearchSymbols(params, storage);
		};
	}
	else if (handlerName == "graph_query") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			service::GraphQueryParams params;

			// Extract symbolFullNames array
			const Json::Value& symbols = args["symbolFullNames"];
			if (symbols.isArray()) {
				for (const auto& symbol : symbols) {
					params.symbolFullNames.push_back(symbol.asString());
				}
			}

			params.maxDepth = args.get("maxDepth", 5).asInt();

			std::wstring plantUML = service::generateSymbolGraph(params, storage);

			Json::Value response;
			response["diagram"] = to_string(plantUML);
			response["format"] = "plantuml";
			return response;
		};
	}
	else if (handlerName == "custom_graph_query") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			service::GraphQueryParams params;

			// Extract symbolFullNames array
			const Json::Value& symbols = args["symbolFullNames"];
			if (symbols.isArray()) {
				for (const auto& symbol : symbols) {
					params.symbolFullNames.push_back(symbol.asString());
				}
			}

			params.maxDepth = args.get("maxDepth", 5).asInt();

			// Extract nodeTypes array
			const Json::Value& nodeTypes = args.get("nodeTypes", Json::Value(Json::arrayValue));
			if (nodeTypes.isArray()) {
				for (const auto& nodeType : nodeTypes) {
					params.nodeTypes.push_back(nodeType.asString());
				}
			}

			// Extract edgeTypes array
			const Json::Value& edgeTypes = args.get("edgeTypes", Json::Value(Json::arrayValue));
			if (edgeTypes.isArray()) {
				for (const auto& edgeType : edgeTypes) {
					params.edgeTypes.push_back(edgeType.asString());
				}
			}

			std::wstring graphOutput = service::generateCustomGraph(params, storage);

			Json::Value response;
			response["diagram"] = to_string(graphOutput);
			return response;
		};
	}
	else if (handlerName == "get_code") {
		return [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			service::CodeQueryParams params;
			params.filePath = FilePath(to_wstring(args["codeFilePath"].asString()));
			params.startLine = args.get("startLine", -1).asInt();
			params.endLine = args.get("endLine", -1).asInt();

			auto result = service::getCodeContent(params, storage);

			Json::Value response;
			if (result.success) {
				response["success"] = true;
				response["code"] = result.content;
			} else {
				response["success"] = false;
				response["error"] = result.errorMessage;
			}
			return response;
		};
	}

	return nullptr;  // Unknown handler
}

void MCPToolRegistry::registerTool(const ToolDefinition& def, ToolHandler handler) {
	toolDefinitions_[def.name] = def;
	toolHandlers_[def.name] = handler;
}

std::vector<ToolDefinition> MCPToolRegistry::getAllTools() const {
	std::vector<ToolDefinition> tools;
	for (const auto& pair : toolDefinitions_) {
		tools.push_back(pair.second);
	}
	return tools;
}

Json::Value MCPToolRegistry::executeTool(const std::string& name, const Json::Value& arguments, StorageAccess* storage) const {
	auto it = toolHandlers_.find(name);
	if (it == toolHandlers_.end()) {
		throw std::runtime_error("Tool not found: " + name);
	}
	return it->second(arguments, storage);
}

bool MCPToolRegistry::hasTool(const std::string& name) const {
	return toolHandlers_.find(name) != toolHandlers_.end();
}

}} // namespace
