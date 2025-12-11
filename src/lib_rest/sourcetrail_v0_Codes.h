#pragma once

#include <drogon/HttpController.h>

struct CodesQueryRequest
{
    std::string codeFilePath;
    int startLine = -1;
    int endLine = -1;
};

namespace drogon
{
    template <>
    inline CodesQueryRequest fromRequest(const HttpRequest& req)
    {
        auto json = req.getJsonObject();
        CodesQueryRequest queryReq;
        if (json == nullptr)
        {
            return queryReq;
        }
        if (json->isMember("codeFilePath"))
        {
            queryReq.codeFilePath = (*json)["codeFilePath"].asString();
        }
        if (json->isMember("startLine"))
        {
            queryReq.startLine = (*json)["startLine"].asInt();
        }
        if (json->isMember("endLine"))
        {
            queryReq.endLine = (*json)["endLine"].asInt();
        }
        return queryReq;
    }
} // namespace drogon

namespace sourcetrail
{
namespace v0
{
class Codes : public drogon::HttpController<Codes>
{
public:
    METHOD_LIST_BEGIN
      METHOD_ADD(Codes::query_codes, "/", drogon::Post);
    METHOD_LIST_END

private:
    void query_codes(const CodesQueryRequest&& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) const;
};
}
}
