//
//
#include "Precompiled.h"
#include "HTTPServer.h"

int wmain(int argc, wchar_t **argv)
{
	HTTPServer server;
	if (server.InitializeServer(L"http://localhost:10240/") == NO_ERROR) {
		server.DoReceiveRequests();
	}
	return 0;
}
