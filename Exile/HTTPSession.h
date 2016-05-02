//
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H
#include <cpprest/containerstream.h> // Async streams backed by STL containers</span>
#include <cpprest/http_listener.h>
#include <cpprest/interopstream.h> // Bridges for integrating Async streams with STL and WinRT streams</span>
#include <cpprest/producerconsumerstream.h> // Async streams for producer consumer scenarios</span>
#include <cpprest/rawptrstream.h> // Async streams backed by raw pointer to memory</span>



#define PIPE_BUFFER_SIZE 0x2000 /// 8192


using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

enum GitSmartSessionChannel {
  kGitUploadPackLs,
  kGitUploadPackStream,
  kGitReceivePackLs,
  kGitReceivePackStream
};

class GitSmartSession {
public:
  GitSmartSession();
};
/*
* git upload-pack
*/

class HTTPSession {
public:
  explicit HTTPSession(http_request &request) : request_(request) {
  }
  bool Execute(const std::wstring &relativePath, int channel);
private:
  http_request &request_;
  http_response response_;
  int lastError;
};

#endif
