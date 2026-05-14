#include "rest_api_main.h"
#include <drogon/HttpAppFramework.h>
#include <thread>
#include <memory>
#include <StorageAccess.h>

void rest_api_woker(int port)
{
	drogon::app().addListener("0.0.0.0", port);
	//drogon::app().loadConfigFile("config.json");
	drogon::app().run();
}

static StorageAccess* s_storageInstance = nullptr;
static std::unique_ptr<std::thread> s_restApiThread = nullptr;

StorageAccess* getStorageInstance()
{
	return s_storageInstance;
}

void startup_rest_api_server(StorageAccess* storageInstance, int port)
{
	s_storageInstance = storageInstance;
	s_restApiThread = std::make_unique<std::thread>(rest_api_woker, port);
}

void shutdown_rest_api_server()
{
	if (s_restApiThread && s_restApiThread->joinable())
	{
		drogon::app().quit();
		s_restApiThread->join();
		s_restApiThread.reset();
	}
}
