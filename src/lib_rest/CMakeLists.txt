
add_files(
	LIB_REST

	rest_util.h

	rest_api_main.h
	rest_api_main.cpp

    sourcetrail_v0_Codes.h
    sourcetrail_v0_Codes.cc

	sourcetrail_v0_Projects.h
	sourcetrail_v0_Projects.cc

	sourcetrail_v0_Symbols.cc
	sourcetrail_v0_Symbols.h

	# Service layer (shared business logic)
	service/ProjectService.h
	service/ProjectService.cpp
	service/SymbolService.h
	service/SymbolService.cpp
	service/CodeService.h
	service/CodeService.cpp

	# MCP protocol
	mcp/JsonRpcProtocol.h
	mcp/JsonRpcProtocol.cpp
	mcp/MCPToolRegistry.h
	mcp/MCPToolRegistry.cpp
	mcp/MCPController.h
	mcp/MCPController.cpp
)
