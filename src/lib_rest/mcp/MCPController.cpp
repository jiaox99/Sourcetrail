#include "MCPController.h"
#include "rest_api_main.h"

namespace sourcetrail {
namespace mcp {

void MCP::handleMessages(const drogon::HttpRequestPtr& req,
						std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	// Parse JSON-RPC request
	auto jsonBody = req->getJsonObject();
	if (!jsonBody) {
		auto errorResp = JsonRpcProtocol::formatError(
			Json::Value(Json::nullValue),
			JsonRpcProtocol::PARSE_ERROR,
			"Invalid JSON"
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
		return;
	}

	JsonRpcRequest rpcRequest = JsonRpcProtocol::parseRequest(*jsonBody);
	if (!rpcRequest.valid) {
		auto errorResp = JsonRpcProtocol::formatError(
			rpcRequest.id,
			JsonRpcProtocol::INVALID_REQUEST,
			"Invalid JSON-RPC 2.0 request"
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
		return;
	}

	// Route by method
	if (rpcRequest.method == "initialize") {
		handleInitialize(rpcRequest, std::move(callback));
	} else if (rpcRequest.method == "tools/list") {
		handleToolsList(rpcRequest, std::move(callback));
	} else if (rpcRequest.method == "tools/call") {
		handleToolsCall(rpcRequest, std::move(callback));
	} else {
		auto errorResp = JsonRpcProtocol::formatError(
			rpcRequest.id,
			JsonRpcProtocol::METHOD_NOT_FOUND,
			"Method not found: " + rpcRequest.method
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
	}
}

void MCP::handleInitialize(const JsonRpcRequest& request,
						  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	// Build server capabilities response per MCP protocol
	Json::Value capabilities;
	capabilities["tools"] = Json::objectValue;  // We support tools

	Json::Value serverInfo;
	serverInfo["name"] = "Sourcetrail MCP Server";
	serverInfo["version"] = "1.0.0";

	Json::Value result;
	result["protocolVersion"] = "2024-11-05";  // MCP protocol version
	result["capabilities"] = capabilities;
	result["serverInfo"] = serverInfo;

	JsonRpcResponse response;
	response.id = request.id;
	response.result = result;

	auto jsonResp = JsonRpcProtocol::formatResponse(response);
	callback(drogon::HttpResponse::newHttpJsonResponse(jsonResp));
}

void MCP::handleToolsList(const JsonRpcRequest& request,
						 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	Json::Value tools(Json::arrayValue);
	for (const auto& tool : registry_.getAllTools()) {
		Json::Value toolJson;
		toolJson["name"] = tool.name;
		toolJson["description"] = tool.description;
		toolJson["inputSchema"] = tool.inputSchema;
		tools.append(toolJson);
	}

	Json::Value result;
	result["tools"] = tools;

	JsonRpcResponse response;
	response.id = request.id;
	response.result = result;

	auto jsonResp = JsonRpcProtocol::formatResponse(response);
	callback(drogon::HttpResponse::newHttpJsonResponse(jsonResp));
}

void MCP::handleToolsCall(const JsonRpcRequest& request,
						 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
	// Extract tool name and arguments
	if (!request.params.isMember("name")) {
		auto errorResp = JsonRpcProtocol::formatError(
			request.id,
			JsonRpcProtocol::INVALID_PARAMS,
			"Missing 'name' parameter"
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
		return;
	}

	std::string toolName = request.params["name"].asString();
	Json::Value arguments = request.params.get("arguments", Json::Value(Json::objectValue));

	if (!registry_.hasTool(toolName)) {
		auto errorResp = JsonRpcProtocol::formatError(
			request.id,
			JsonRpcProtocol::INVALID_PARAMS,
			"Tool not found: " + toolName
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
		return;
	}

	try {
		// Execute tool
		StorageAccess* storage = getStorageInstance();
		Json::Value toolResult = registry_.executeTool(toolName, arguments, storage);

		// Wrap in MCP content format
		Json::Value content(Json::arrayValue);
		Json::Value textContent;
		textContent["type"] = "text";
		textContent["text"] = Json::FastWriter().write(toolResult);
		content.append(textContent);

		Json::Value result;
		result["content"] = content;

		JsonRpcResponse response;
		response.id = request.id;
		response.result = result;

		auto jsonResp = JsonRpcProtocol::formatResponse(response);
		callback(drogon::HttpResponse::newHttpJsonResponse(jsonResp));

	} catch (const std::exception& e) {
		auto errorResp = JsonRpcProtocol::formatError(
			request.id,
			JsonRpcProtocol::INTERNAL_ERROR,
			std::string("Tool execution failed: ") + e.what()
		);
		callback(drogon::HttpResponse::newHttpJsonResponse(errorResp));
	}
}

}} // namespace
