//
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H
#include <cpprest/http_listener.h>
#include <cpprest/containerstream.h>            // Async streams backed by STL containers</span>
#include <cpprest/interopstream.h>              // Bridges for integrating Async streams with STL and WinRT streams</span>
#include <cpprest/rawptrstream.h>               // Async streams backed by raw pointer to memory</span>
#include <cpprest/producerconsumerstream.h>     // Async streams for producer consumer scenarios</span>

#define MAX_UNC_PATH (1024*32-1)
#define PIPE_BUFFER_SIZE 4096
const WCHAR* GetRepositoriesRoot();

using namespace web;
using namespace http;
using namespace utility;
// using http listener
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
	explicit HTTPSession(http_request &request) 
		:request_(request)
	{
		//
	}
	bool Execute(const std::wstring &relativePath, int channel);
private:
	http_request &request_;
	http_response response_;
};


#endif
