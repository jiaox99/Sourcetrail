#pragma once
#include <string>
#include "FilePath.h"

class StorageAccess;

namespace sourcetrail {
namespace service {

struct CodeQueryParams {
	FilePath filePath;
	int startLine = -1;  // -1 means from beginning
	int endLine = -1;    // -1 means to end
};

struct CodeQueryResult {
	bool success;
	std::string content;
	std::string errorMessage;
};

// Retrieve source code content
CodeQueryResult getCodeContent(const CodeQueryParams& params, StorageAccess* storage);

}} // namespace
