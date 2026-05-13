#pragma once
#include <drogon/HttpController.h>
#include "MCPToolRegistry.h"
#include "JsonRpcProtocol.h"

namespace sourcetrail {
namespace mcp {

class MCP : public drogon::HttpController<MCP> {
public:
	METHOD_LIST_BEGIN
		METHOD_ADD(MCP::handleMessages, "/messages", drogon::Post);
		// Note: SSE endpoint can be added later if needed
	METHOD_LIST_END

private:
	void handleMessages(const drogon::HttpRequestPtr& req,
					   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

	void handleToolsList(const JsonRpcRequest& request,
						std::function<void(const drogon::HttpResponsePtr&)>&& callback);

	void handleToolsCall(const JsonRpcRequest& request,
						std::function<void(const drogon::HttpResponsePtr&)>&& callback);

	MCPToolRegistry registry_;
};

}} // namespace
