#include "JsonRpcProtocol.h"

namespace sourcetrail {
namespace mcp {

JsonRpcRequest JsonRpcProtocol::parseRequest(const Json::Value& json) {
	JsonRpcRequest req;

	// Validate structure
	if (!json.isObject()) {
		return req;  // valid=false
	}

	if (!json.isMember("jsonrpc") || json["jsonrpc"].asString() != "2.0") {
		return req;
	}

	if (!json.isMember("method") || !json["method"].isString()) {
		return req;
	}

	req.jsonrpc = "2.0";
	req.method = json["method"].asString();
	req.params = json.get("params", Json::Value(Json::objectValue));
	req.id = json.get("id", Json::Value(Json::nullValue));
	req.valid = true;

	return req;
}

Json::Value JsonRpcProtocol::formatResponse(const JsonRpcResponse& resp) {
	Json::Value json;
	json["jsonrpc"] = resp.jsonrpc;
	json["id"] = resp.id;

	if (!resp.error.isNull()) {
		json["error"] = resp.error;
	} else {
		json["result"] = resp.result;
	}

	return json;
}

Json::Value JsonRpcProtocol::formatError(const Json::Value& id, int code, const std::string& message) {
	Json::Value error;
	error["code"] = code;
	error["message"] = message;

	JsonRpcResponse resp;
	resp.id = id;
	resp.error = error;

	return formatResponse(resp);
}

}} // namespace
