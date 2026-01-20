#pragma once

#include <drogon/HttpController.h>
#include <boost/locale.hpp>

inline std::wstring to_wstring(const std::string& str)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(str);
}

inline std::string to_string(const std::wstring& wstr)
{
	return boost::locale::conv::utf_to_utf<char>(wstr);
}

inline void message_response(const std::string& message, std::function<void(const drogon::HttpResponsePtr&)>& callback)
{
	auto resp = drogon::HttpResponse::newHttpResponse();
	resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
	resp->setStatusCode(drogon::k200OK);
	resp->setBody(message);
	callback(resp);
}