#include "sourcetrail_v0_Codes.h"
#include "service/CodeService.h"
#include "rest_api_main.h"
#include "rest_util.h"
#include "FilePath.h"

using namespace sourcetrail::v0;

void sourcetrail::v0::Codes::query_codes(
	const CodesQueryRequest&& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	StorageAccess* storage = getStorageInstance();

	// Use service layer
	sourcetrail::service::CodeQueryParams params;
	params.filePath = FilePath(to_wstring(req.codeFilePath));
	params.startLine = req.startLine;
	params.endLine = req.endLine;

	auto result = sourcetrail::service::getCodeContent(params, storage);

	if (result.success) {
		message_response(result.content, callback);
	} else {
		message_response(result.errorMessage, callback);
	}
}
