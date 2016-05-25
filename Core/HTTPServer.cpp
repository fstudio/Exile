///
#include "Precompiled.h"
#include <Http.h>
#include <httpserv.h>
#include <Mswsock.h>
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

#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )                    \
    do                                                                      \
	    {                                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );                           \
        (resp)->StatusCode = (status);                                      \
        (resp)->pReason = (reason);                                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);                     \
	    } while (FALSE)



#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)                      \
    do                                                                      \
	    {                                                                       \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue = (RawValue); \
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength =        \
            (USHORT) strlen(RawValue);                                      \
	    } while(FALSE)

template<class T>
inline T* Allocate(size_t n)
{
	return reinterpret_cast<T*>(HeapAlloc(GetProcessHeap(), 0, n*sizeof(T)));
}

#define MemoryFree(ptr) HeapFree(GetProcessHeap(), 0, (ptr))

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

ULONG HTTPServer::InitializeServer(LPCWSTR url)
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
DWORD HTTPServer::DoReceiveRequests()
{
	ULONG              result;
	HTTP_REQUEST_ID    requestId;
	DWORD              bytesRead;
	PHTTP_REQUEST      pRequest;
	PCHAR              pRequestBuffer;
	ULONG              RequestBufferLength;

	//
	// Allocate a 2K buffer. Should be good for most requests, we'll grow
	// this if required. We also need space for a HTTP_REQUEST structure.
	//
	RequestBufferLength = sizeof(HTTP_REQUEST) + 2048;
	pRequestBuffer = Allocate<CHAR>(RequestBufferLength);

	if (pRequestBuffer == NULL) {
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	pRequest = (PHTTP_REQUEST)pRequestBuffer;

	//
	// Wait for a new request -- This is indicated by a NULL request ID.
	//

	HTTP_SET_NULL_ID(&requestId);

	for (;;) {
		RtlZeroMemory(pRequest, RequestBufferLength);

		result = HttpReceiveHttpRequest(
			hRequestQueue,          // Req Queue
			requestId,          // Req ID
			0,                  // Flags
			pRequest,           // HTTP request buffer
			RequestBufferLength,// req buffer length
			&bytesRead,         // bytes received
			NULL                // LPOVERLAPPED
			);

		if (NO_ERROR == result) {
			//
			// Worked!
			//
			switch (pRequest->Verb) {
				case HttpVerbGET:
				wprintf(L"Got a GET request for %ws \n",
						pRequest->CookedUrl.pFullUrl);
				//result = SendHttpResponse(
				//	hRequestQueue,
				//	pRequest,
				//	200,
				//	"OK",
				//	"Hey! You hit the server \r\n"
				//	);
				break;

				case HttpVerbPOST:

				wprintf(L"Got a POST request for %ws \n",
						pRequest->CookedUrl.pFullUrl);

				//result = SendHttpPostResponse(hRequestQueue, pRequest);
				break;

				default:
				wprintf(L"Got a unknown request for %ws \n",
						pRequest->CookedUrl.pFullUrl);

				//result = SendHttpResponse(
				//	hRequestQueue,
				//	pRequest,
				//	503,
				//	"Not Implemented",
				//	NULL
				//	);
				break;
			}

			if (result != NO_ERROR) {
				break;
			}

			//
			// Reset the Request ID so that we pick up the next request.
			//
			HTTP_SET_NULL_ID(&requestId);
		} else if (result == ERROR_MORE_DATA) {
			//
			// The input buffer was too small to hold the request headers
			// We have to allocate more buffer & call the API again.
			//
			// When we call the API again, we want to pick up the request
			// that just failed. This is done by passing a RequestID.
			//
			// This RequestID is picked from the old buffer.
			//
			requestId = pRequest->RequestId;

			//
			// Free the old buffer and allocate a new one.
			//
			RequestBufferLength = bytesRead;
			MemoryFree(pRequestBuffer);
			pRequestBuffer = Allocate<CHAR>(RequestBufferLength);

			if (pRequestBuffer == NULL) {
				result = ERROR_NOT_ENOUGH_MEMORY;
				break;
			}

			pRequest = (PHTTP_REQUEST)pRequestBuffer;

		} else if (ERROR_CONNECTION_INVALID == result &&
				   !HTTP_IS_NULL_ID(&requestId)) {
			// The TCP connection got torn down by the peer when we were
			// trying to pick up a request with more buffer. We'll just move
			// onto the next request.

			HTTP_SET_NULL_ID(&requestId);
		} else {
			break;
		}

	} // for(;;)

	if (pRequestBuffer) {
		MemoryFree(pRequestBuffer);
	}

	return result;
}