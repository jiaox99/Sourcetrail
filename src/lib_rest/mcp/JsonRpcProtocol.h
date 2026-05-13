#pragma once
#include <json/json.h>
#include <string>

namespace sourcetrail {
namespace mcp {

struct JsonRpcRequest {
	std::string jsonrpc;  // Must be "2.0"
	std::string method;   // e.g., "tools/list", "tools/call"
	Json::Value params;
	Json::Value id;
	bool valid = false;
};

struct JsonRpcResponse {
	std::string jsonrpc = "2.0";
	Json::Value result;   // Mutually exclusive with error
	Json::Value error;    // Mutually exclusive with result
	Json::Value id;
};

class JsonRpcProtocol {
public:
	// Parse JSON-RPC request from JSON object
	static JsonRpcRequest parseRequest(const Json::Value& json);

	// Build JSON-RPC response
	static Json::Value formatResponse(const JsonRpcResponse& resp);

	// Build JSON-RPC error
	static Json::Value formatError(const Json::Value& id, int code, const std::string& message);

	// Standard JSON-RPC error codes
	static constexpr int PARSE_ERROR = -32700;
	static constexpr int INVALID_REQUEST = -32600;
	static constexpr int METHOD_NOT_FOUND = -32601;
	static constexpr int INVALID_PARAMS = -32602;
	static constexpr int INTERNAL_ERROR = -32603;
};

}} // namespace
