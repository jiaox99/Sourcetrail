#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

namespace sourcetrail
{
namespace v0
{
class Symbols : public drogon::HttpSimpleController<Symbols>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
	PATH_ADD("/", Get);
    PATH_ADD("/symbols", Get);
    PATH_LIST_END
};
}
}
