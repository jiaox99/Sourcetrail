#include "CodeService.h"
#include "StorageAccess.h"
#include "TextAccess.h"

namespace sourcetrail {
namespace service {

CodeQueryResult getCodeContent(const CodeQueryParams& params, StorageAccess* storage) {
	if (storage == nullptr) {
		return {false, "", "Storage not initialized"};
	}

	if (params.filePath.empty() || !params.filePath.exists()) {
		return {false, "", "File not found: " + params.filePath.str()};
	}

	std::shared_ptr<TextAccess> textAccess = storage->getFileContent(params.filePath, false);
	if (!textAccess || textAccess->isEmpty()) {
		return {false, "", "No content for file: " + params.filePath.str()};
	}

	std::string code;
	int startLine = params.startLine > 0 ? params.startLine : 1;
	int endLine = params.endLine > 0 ? params.endLine : textAccess->getLineCount();

	if (startLine > endLine || startLine < 1 || endLine > textAccess->getLineCount()) {
		// Invalid range, return entire file
		code = textAccess->getText();
	} else {
		// Extract line range
		auto lines = textAccess->getLines((unsigned int)startLine, (unsigned int)endLine);
		for (const auto& line : lines) {
			code += line;
		}
	}

	return {true, code, ""};
}

}} // namespace
