#include "MCPToolRegistry.h"
#include "service/ProjectService.h"
#include "service/SymbolService.h"
#include "service/CodeService.h"
#include "rest_util.h"
#include "FilePath.h"

namespace sourcetrail {
namespace mcp {

MCPToolRegistry::MCPToolRegistry() {
	registerTools();
}

void MCPToolRegistry::registerTools() {
	// Tool 1: sourcetrail_list_projects
	{
		ToolDefinition def;
		def.name = "sourcetrail_list_projects";
		def.description = "List recently opened Sourcetrail projects and show the currently loaded project";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"] = Json::Value(Json::objectValue);
		def.inputSchema["required"] = Json::Value(Json::arrayValue);

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			return service::listProjects();
		};

		registerTool(def, handler);
	}

	// Tool 2: sourcetrail_load_project
	{
		ToolDefinition def;
		def.name = "sourcetrail_load_project";
		def.description = "Load a Sourcetrail project file (.srctrlprj) for querying";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"]["projectFilePath"]["type"] = "string";
		def.inputSchema["properties"]["projectFilePath"]["description"] = "Absolute path to .srctrlprj file";
		def.inputSchema["required"].append("projectFilePath");

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			std::string pathStr = args["projectFilePath"].asString();
			FilePath path(pathStr);
			auto result = service::loadProject(path);

			Json::Value response;
			response["success"] = result.success;
			response["message"] = result.message;
			return response;
		};

		registerTool(def, handler);
	}

	// Tool 3: sourcetrail_fuzzy_search
	{
		ToolDefinition def;
		def.name = "sourcetrail_fuzzy_search";
		def.description = "Search for symbols (classes, functions, variables, etc.) in the loaded project using fuzzy matching";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"]["query"]["type"] = "string";
		def.inputSchema["properties"]["query"]["description"] = "Search query string";
		def.inputSchema["properties"]["maxResults"]["type"] = "integer";
		def.inputSchema["properties"]["maxResults"]["description"] = "Maximum number of results to return (default: 50)";
		def.inputSchema["required"].append("query");

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
			service::FuzzySearchParams params;
			params.query = to_wstring(args["query"].asString());
			params.maxResults = args.get("maxResults", 50).asInt();

			return service::fuzzySearchSymbols(params, storage);
		};

		registerTool(def, handler);
	}

	// Tool 4: sourcetrail_graph_query
	{
		ToolDefinition def;
		def.name = "sourcetrail_graph_query";
		def.description = "Generate a graph showing relationships for a symbol (inheritance for classes, calls for functions, includes for files). Returns PlantUML diagram format.";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"]["symbolFullNames"]["type"] = "array";
		def.inputSchema["properties"]["symbolFullNames"]["description"] = "Array of serialized symbol names (use serializedName from fuzzy_search results)";
		def.inputSchema["properties"]["symbolFullNames"]["items"]["type"] = "string";
		def.inputSchema["properties"]["maxDepth"]["type"] = "integer";
		def.inputSchema["properties"]["maxDepth"]["description"] = "Maximum depth for graph traversal (default: 5)";
		def.inputSchema["required"].append("symbolFullNames");

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
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

		registerTool(def, handler);
	}

	// Tool 5: sourcetrail_custom_graph_query
	{
		ToolDefinition def;
		def.name = "sourcetrail_custom_graph_query";
		def.description = "Find paths between two symbols with optional filtering by node types (class, function, field) and edge types (call, inheritance, usage)";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"]["symbolFullNames"]["type"] = "array";
		def.inputSchema["properties"]["symbolFullNames"]["description"] = "Exactly two serialized symbol names: [origin, target]";
		def.inputSchema["properties"]["symbolFullNames"]["items"]["type"] = "string";
		def.inputSchema["properties"]["maxDepth"]["type"] = "integer";
		def.inputSchema["properties"]["maxDepth"]["description"] = "Maximum path depth (default: 5)";
		def.inputSchema["properties"]["nodeTypes"]["type"] = "array";
		def.inputSchema["properties"]["nodeTypes"]["description"] = "Filter by node types (e.g., ['class', 'function']). Empty means all types.";
		def.inputSchema["properties"]["nodeTypes"]["items"]["type"] = "string";
		def.inputSchema["properties"]["edgeTypes"]["type"] = "array";
		def.inputSchema["properties"]["edgeTypes"]["description"] = "Filter by edge types (e.g., ['call', 'inheritance']). Empty means all types.";
		def.inputSchema["properties"]["edgeTypes"]["items"]["type"] = "string";
		def.inputSchema["required"].append("symbolFullNames");

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
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

		registerTool(def, handler);
	}

	// Tool 6: sourcetrail_get_code
	{
		ToolDefinition def;
		def.name = "sourcetrail_get_code";
		def.description = "Retrieve source code content from a file, optionally specifying a line range";
		def.inputSchema["type"] = "object";
		def.inputSchema["properties"]["codeFilePath"]["type"] = "string";
		def.inputSchema["properties"]["codeFilePath"]["description"] = "Absolute path to source code file";
		def.inputSchema["properties"]["startLine"]["type"] = "integer";
		def.inputSchema["properties"]["startLine"]["description"] = "Starting line number (1-indexed). Omit or use -1 for start of file.";
		def.inputSchema["properties"]["endLine"]["type"] = "integer";
		def.inputSchema["properties"]["endLine"]["description"] = "Ending line number (inclusive). Omit or use -1 for end of file.";
		def.inputSchema["required"].append("codeFilePath");

		ToolHandler handler = [](const Json::Value& args, StorageAccess* storage) -> Json::Value {
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

		registerTool(def, handler);
	}
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
