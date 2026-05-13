#pragma once
#include <json/json.h>
#include <string>
#include <functional>
#include <map>
#include <vector>

class StorageAccess;

namespace sourcetrail {
namespace mcp {

struct ToolDefinition {
	std::string name;
	std::string description;
	Json::Value inputSchema;  // JSON Schema object
};

using ToolHandler = std::function<Json::Value(const Json::Value& arguments, StorageAccess*)>;

class MCPToolRegistry {
public:
	MCPToolRegistry();

	// Get all tool definitions for tools/list
	std::vector<ToolDefinition> getAllTools() const;

	// Execute a tool by name
	Json::Value executeTool(const std::string& name, const Json::Value& arguments, StorageAccess* storage) const;

	// Check if tool exists
	bool hasTool(const std::string& name) const;

private:
	void registerTools();
	void registerTool(const ToolDefinition& def, ToolHandler handler);

	std::map<std::string, ToolDefinition> toolDefinitions_;
	std::map<std::string, ToolHandler> toolHandlers_;
};

}} // namespace
