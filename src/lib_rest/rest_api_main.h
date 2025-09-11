#ifndef REST_API_MAIN_H
#define REST_API_MAIN_H
class StorageAccess;
StorageAccess* getStorageInstance();
void startup_rest_api_server(StorageAccess* storageInstance, int port);
void shutdown_rest_api_server();
#endif // !REST_API_MAIN_H
