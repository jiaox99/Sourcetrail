#include "sourcetrail_v0_Codes.h"
#include "rest_api_main.h"
#include "rest_util.h"
#include "FilePath.h"
#include "TextAccess.h"
#include "StorageAccess.h"

using namespace sourcetrail::v0;

void sourcetrail::v0::Codes::query_codes(
	const CodesQueryRequest&& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	StorageAccess* storage = getStorageInstance();
	if (!storage) {
		message_response("Storage not initialized", callback);
		return;
	}

	FilePath filePath(to_wstring(req.codeFilePath));
	if (filePath.empty() || !filePath.exists()) {
		message_response("File not found: " + req.codeFilePath, callback);
		return;
	}

	std::shared_ptr<TextAccess> textAccess = storage->getFileContent(filePath, false);
	if (!textAccess || textAccess->isEmpty()) {
		message_response("No content for file: " + req.codeFilePath, callback);
		return;
	}

	std::string code;
	int startLine = req.startLine > 0 ? req.startLine : 1;
	int endLine = req.endLine > 0 ? req.endLine : textAccess->getLineCount();
	if (startLine > endLine || startLine < 1 || endLine > textAccess->getLineCount())
	{
		code = textAccess->getText();
	}
	else
	{
		auto lines = textAccess->getLines((unsigned int)startLine, (unsigned int)endLine);
		for (const auto& line: lines)
		{
			code += line;
		}
	}

	message_response(code, callback);
}
