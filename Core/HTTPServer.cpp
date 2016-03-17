///
#include "Precompiled.h"
#include <Http.h>
#include "HTTPServer.h"


/*
* What's New for Windows Sockets
* https://msdn.microsoft.com/library/windows/desktop/ms740642(v=vs.85).aspx
* Registered Input/Output (RIO) API Extensions
* https://technet.microsoft.com/en-us/library/hh997032.aspx
* Mswsock.h Mswsock.lib Mswsock.dll Windows 8/ Windows Server 2012 or Later
* https://msdn.microsoft.com/en-us/library/windows/desktop/hh448841(v=vs.85).aspx
* RIOCreateCompletionQueue
* http://www.cnblogs.com/gaochundong/p/csharp_tcp_service_models.html
*/

/*
* HTTP2 Allow http.h HTTP_REQUEST_FLAG_HTTP2
* httpserv.h:1093:// Add the SPDY/3 & HTTP/2.0 Push-Promise support
*/

HTTPServer::HTTPServer()
{
	HTTPAPI_VERSION httpApiVersion = HTTPAPI_VERSION_2;
	HttpInitialize(httpApiVersion, HTTP_INITIALIZE_SERVER, NULL);
}

HTTPServer::~HTTPServer()
{
	if (!HTTP_IS_NULL_ID(&m_urlGroupId)) {
		HttpRemoveUrlFromUrlGroup(m_urlGroupId,
								  NULL,
								  HTTP_URL_FLAG_REMOVE_ALL);
	}
	if (!HTTP_IS_NULL_ID(&m_urlGroupId)) {
		if (HttpCloseUrlGroup(m_urlGroupId) != NO_ERROR) {
			/// show
		}
	}
	if (!HTTP_IS_NULL_ID(&m_serverSessionId)) {
		if (HttpCloseServerSession(m_serverSessionId) != NO_ERROR) {
			//
		}
	}

	if (hRequestQueue) {
		if (HttpCloseRequestQueue(hRequestQueue) != NO_ERROR) {
			///
		}
	}
	HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
}

ULONG HTTPServer::InitializeHTTPServer(LPCWSTR url)
{
	auto retCode = HttpCreateServerSession(HTTPAPI_VERSION_2,&m_serverSessionId,0);
	if (retCode != NO_ERROR) {
		fprintf(stderr, "HttpCreateServerSession failed ,result: %lu\n",retCode);
		return retCode;
	}
	retCode = HttpCreateUrlGroup(m_serverSessionId,
								 &m_urlGroupId,
								 0);
	if (retCode != NO_ERROR) {
		wprintf(L"HttpCreateUrlGroup failed with %lu \n", retCode);
		return retCode;
	}
	retCode = HttpCreateRequestQueue(HTTPAPI_VERSION_2,
									 L"HTTPServerQueue",
									 NULL,
									 0,
									 &hRequestQueue);
	if (retCode != NO_ERROR) {
		wprintf(L"HttpCreateRequestQueue failed with %lu \n", retCode);
		return retCode;
	}

	BindingProperty.Flags.Present = 1;// Specifies that the property is present on UrlGroup
	BindingProperty.RequestQueueHandle = hRequestQueue;
	retCode = HttpSetUrlGroupProperty(m_urlGroupId,
									  HttpServerBindingProperty,
									  &BindingProperty,
									  sizeof(BindingProperty));

	if (retCode != NO_ERROR) {
		wprintf(L"HttpSetUrlGroupProperty failed with %lu \n", retCode);
		return retCode;
	}


	//
	// Set EntityBody Timeout property on UrlGroup
	//

	ZeroMemory(&CGTimeout, sizeof(HTTP_TIMEOUT_LIMIT_INFO));

	CGTimeout.Flags.Present = 1; // Specifies that the property is present on UrlGroup
	CGTimeout.EntityBody = 50; //The timeout is in secs


	retCode = HttpSetUrlGroupProperty(m_urlGroupId,
									  HttpServerTimeoutsProperty,
									  &CGTimeout,
									  sizeof(HTTP_TIMEOUT_LIMIT_INFO));

	if (retCode != NO_ERROR) {
		wprintf(L"HttpSetUrlGroupProperty failed with %lu \n", retCode);
		return retCode;
	}
	// See MSDN https://msdn.microsoft.com/en-us/library/windows/desktop/aa364698(v=vs.85).aspx
	retCode = HttpAddUrlToUrlGroup(m_urlGroupId,
								   url,
								   0,
								   0);
	if (retCode != NO_ERROR) {
		wprintf(L"HttpAddUrl failed with %lu \n", retCode);
		return retCode;
	}
	return NO_ERROR;
}
void HTTPServer::DoReceiveRequests()
{
	for (;;) {

	}
}