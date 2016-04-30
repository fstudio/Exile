//
#include "Authorization.h"
#include "HTTPServer.h"
#include "HTTPSession.h"
#include "Precompiled.h"


// http://blogs.msdn.com/b/varunm/archive/2013/04/23/remove-unwanted-http-response-headers.aspx
// HKLM\SYSTEM\CurrentControlSet\Services\HTTP\Parameters  DisableServerHeader 1

#define PRINT_FUNC printf("Function: " __FUNCTION__ " \n");

HTTPServer::HTTPServer(string_t url) : m_listener(url) {
  m_listener.support(methods::GET,
                     std::bind(&HTTPServer::Get, this, std::placeholders::_1));
  m_listener.support(methods::PUT,
                     std::bind(&HTTPServer::Put, this, std::placeholders::_1));
  m_listener.support(methods::POST,
                     std::bind(&HTTPServer::Post, this, std::placeholders::_1));
  m_listener.support(methods::DEL, std::bind(&HTTPServer::Delete, this,
                                             std::placeholders::_1));
  //// default handle
  m_listener.support(methods::HEAD, std::bind(&HTTPServer::Default, this,
                                              std::placeholders::_1));
  m_listener.support(methods::CONNECT, std::bind(&HTTPServer::Default, this,
                                                 std::placeholders::_1));
  m_listener.support(methods::MERGE, std::bind(&HTTPServer::Default, this,
                                               std::placeholders::_1));
  m_listener.support(methods::OPTIONS, std::bind(&HTTPServer::Default, this,
                                                 std::placeholders::_1));
  m_listener.support(methods::PATCH, std::bind(&HTTPServer::Default, this,
                                               std::placeholders::_1));
  m_listener.support(methods::TRCE, std::bind(&HTTPServer::Default, this,
                                              std::placeholders::_1));
}

//#define USERAGENT_ACCESS_CHECK

bool RequestFilter(http_request &req) {
  auto &m_header = req.headers();
#ifdef USERAGENT_ACCESS_CHECK
  auto ua = m_header[L"User-Agent"];
  if (ua.compare(0, 4, L"git/") != 0 && ua.compare(0, 5, L"JGIT/") != 0) {
    req.reply(status_codes::Forbidden, L"Access Forbidden");
  }
#endif
  http::http_response resp;
  if (m_header.has(L"Authorization")) {
    auto base64text = m_header[L"Authorization"];
    if (AuthorizedWithBasic(base64text)) {
      return true;
    }
    resp.set_body(L"Disable");
    resp.set_status_code(status_codes::Forbidden);
    req.reply(resp);
    return false;
  }
  resp.set_body("Unauthorization");
  auto &hd = resp.headers();
  /// What's fuck ,cannot set Server name ?
  hd.add(L"Server", L"Horatio/1.0");
  hd.add(L"WWW-Authenticate", LR"(Basic realm="")");
  resp.set_status_code(status_codes::Unauthorized);
  req.reply(resp);
  return false;
}

void HTTPServer::Get(http_request message) {
#ifndef _DEBUG
  if (!RequestFilter(message)) {
    return;
  }
#endif

  auto path = http::uri::decode(message.relative_uri().path());
  DebugMessage(kInfo, L"OPTIONS:  GET %s\n",message.absolute_uri().to_string().data());
  auto &query = uri::split_query(uri::decode(message.request_uri().query()));
  // std::wstring msg;
  string_t relativePath;
  if (path.compare(path.size() - sizeof("/info/refs") + 1,
                   sizeof("/info/refs") - 1, L"/info/refs") == 0) {
    relativePath = path.substr(0, path.size() - sizeof("/info/refs") + 1);
    // msg = L"repository relative path= " + relativePath;
  }
  auto iter = query.find(L"service");
  if (iter != query.end()) {
    HTTPSession httpSession(message);
    auto &cmd = iter->second;
    if (cmd.compare(L"git-upload-pack") == 0) {
      httpSession.Execute(relativePath, kGitUploadPackLs);
    } else if (cmd.compare(L"git-receive-pack") == 0) {
      httpSession.Execute(relativePath, kGitReceivePackLs);
    } else {
    }
    return;
  }
  // message.reply(status_codes::OK,msg);
  
}

void HTTPServer::Put(http_request message) { 
	DebugMessage(kError, L"PUT Method Not Allow ");
}
void HTTPServer::Post(http_request message) {
#ifndef _DEBUG
  if (!RequestFilter(message)) {
    return;
  }
#endif
  auto path = http::uri::decode(message.relative_uri().path());
  DebugMessage(kInfo, L"OPTIONS:  POST %s\n", message.absolute_uri().to_string().data());
  auto end = path.rfind(L'/');
  if (end == path.npos) {
    message.reply(status_codes::BadRequest, L"URL Error !");
    return;
  }
  auto relativePath = path.substr(0, end);
  auto cmd = path.substr(end + 1);
  HTTPSession httpSession(message);
  if (cmd.compare(L"git-upload-pack") == 0) {
    httpSession.Execute(relativePath, kGitUploadPackStream);
  } else if (cmd.compare(L"git-receive-pack") == 0) {
    httpSession.Execute(relativePath, kGitReceivePackStream);
  } else {
    ///
  }

}
void HTTPServer::Delete(http_request message) {
	DebugMessage(kError, L"DELETE Method Not Allow\n");
}
void HTTPServer::Default(http_request message) { 
	DebugMessage(kError, L"%s Method Not Allow\n",message.method().data());
}