#include "rest_api_main.h"
#include <drogon/HttpAppFramework.h>
#include <thread>
#include <StorageAccess.h>

void rest_api_woker(int port)
{
	drogon::app().addListener("0.0.0.0", port);
	//drogon::app().loadConfigFile("config.json");
	drogon::app().run();
}

static StorageAccess* s_storageInstance = nullptr;

StorageAccess* getStorageInstance()
{
	return s_storageInstance;
}

void startup_rest_api_server(StorageAccess* storageInstance, int port)
{
	s_storageInstance = storageInstance;
	static std::thread rest_api_thread(rest_api_woker, port);
}

void shutdown_rest_api_server() {}
