#include "sourcetrail_v0_Symbols.h"
#include "rest_api_main.h"
#include "StorageAccess.h"
#include "boost/locale.hpp"
#include "NodeTypeSet.h"

using namespace sourcetrail::v0;

// Add definition of your processing function here
void Symbols::fuzzyQuery(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& query) const
{
	int limit = req->getOptionalParameter<int>("max").value_or(50);
	auto *storage = getStorageInstance();
	auto result = storage->getAutocompletionMatches(boost::locale::conv::utf_to_utf<wchar_t>(query), NodeTypeSet::all(), false);
	Json::Value jsonResult;
	int count = 0;
	for (const auto& entry: result)
	{
		jsonResult.append(boost::locale::conv::utf_to_utf<char>(entry.getFullName()));
		if (++count >= limit)
			break;
	}
	auto resp = HttpResponse::newHttpJsonResponse(jsonResult);
	callback(resp);
}