//
//
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <Http.h>
#if defined(_WIN32_WINNT_WIN8)&&_WIN32_WINNT >=_WIN32_WINNT_WIN8
#include <Threadpoolapiset.h >
#endif
#include <functional>

#define CHUNK_SIZE 64 * 1024

class http_overlapped : public OVERLAPPED {
public:
	void set_http_io_completion(std::function<void(DWORD, DWORD)> http_io_completion)
	{
		ZeroMemory(this, sizeof(OVERLAPPED));
		m_http_io_completion = http_io_completion;
	}

	/// <summary>
	/// Callback for all I/O completions.
	/// </summary>
	static void CALLBACK io_completion_callback(
		PTP_CALLBACK_INSTANCE instance,
		PVOID context,
		PVOID pOverlapped,
		ULONG result,
		ULONG_PTR numberOfBytesTransferred,
		PTP_IO io)
	{
		//CASABLANCA_UNREFERENCED_PARAMETER(io);
		//CASABLANCA_UNREFERENCED_PARAMETER(context);
		//CASABLANCA_UNREFERENCED_PARAMETER(instance);

		http_overlapped *p_http_overlapped = (http_overlapped *)pOverlapped;
		p_http_overlapped->m_http_io_completion(result, (DWORD)numberOfBytesTransferred);
	}

private:
	std::function<void(DWORD, DWORD)> m_http_io_completion;
};

class HTTPServer {
public:
	HTTPServer();
	~HTTPServer();
	ULONG InitializeHTTPServer(LPCWSTR url);
	void DoReceiveRequests();
private:
	HANDLE hRequestQueue = NULL;
	HTTP_SERVER_SESSION_ID m_serverSessionId = HTTP_NULL_ID;
	HTTP_URL_GROUP_ID m_urlGroupId = HTTP_NULL_ID;
	HTTP_BINDING_INFO BindingProperty;
	HTTP_TIMEOUT_LIMIT_INFO CGTimeout;
	PTP_IO m_threadpool_io= NULL;
};


#endif