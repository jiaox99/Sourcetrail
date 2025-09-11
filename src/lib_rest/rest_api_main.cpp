#include "rest_api_main.h"
#include <drogon/HttpAppFramework.h>
#include <thread>
#include <StorageAccess.h>

void rest_api_woker()
{
	drogon::app().addListener("0.0.0.0", 9984);
	drogon::app().loadConfigFile("config.json");
	drogon::app().run();
}

static StorageAccess* s_storageInstance = nullptr;

StorageAccess* getStorageInstance()
{
	return s_storageInstance;
}

void startup_rest_api_server(StorageAccess* storageInstance)
{
	s_storageInstance = storageInstance;
	static std::thread rest_api_thread(rest_api_woker);
}

void shutdown_rest_api_server() {}
