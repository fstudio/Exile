//
//

#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H
#include <Http.h>
#include <memory>
class HTTPGitSession {
public:
	HTTPGitSession()
	{

	}
	~HTTPGitSession()
	{
		///
	}
private:
	HANDLE hWrite;
	HANDLE hRead;
};
class HTTPSession {
public:
	HTTPSession();
private:
	HTTP_RESPONSE *response_;
	HTTP_REQUEST *request_;
};


#endif
