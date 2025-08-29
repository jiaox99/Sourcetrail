#include "sourcetrail_v0_Symbols.h"

using namespace sourcetrail::v0;

void Symbols::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_TEXT_PLAIN);
	resp->setBody("Hello, World for symbols");
    callback(resp);
}
