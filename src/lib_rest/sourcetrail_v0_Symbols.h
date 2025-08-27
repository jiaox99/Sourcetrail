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
    // list path definitions here;
    // PATH_ADD("/path", "filter1", "filter2", HttpMethod1, HttpMethod2...);
    PATH_LIST_END
};
}
}
