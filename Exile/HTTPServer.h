///
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <cpprest/http_listener.h>
using namespace web;
using namespace http;
using namespace utility;
// using http listener
using namespace http::experimental::listener;

class HTTPServer {
public:
	HTTPServer(string_t url);
	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }
private:
	http_listener m_listener;
	void Get(http_request message);
	void Put(http_request message);
	void Post(http_request message);
	void Delete(http_request message);
	void Default(http_request message);
};
#endif
