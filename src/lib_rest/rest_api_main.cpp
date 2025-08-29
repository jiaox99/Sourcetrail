#include "rest_api_main.h"
#include <drogon/HttpAppFramework.h>
#include <thread>

void rest_api_woker()
{
	drogon::app().addListener("0.0.0.0", 9984);
	drogon::app().loadConfigFile("config.json");
	drogon::app().run();
}

void startup_rest_api_server()
{
	static std::thread rest_api_thread(rest_api_woker);
}

void shutdown_server() {}
