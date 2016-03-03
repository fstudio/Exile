#include "Precompiled.h"
#include "HTTPServer.h"

std::unique_ptr<HTTPServer> g_HTTPServer;
static WCHAR repositoriesRoot[4096] = LR"(F:\GitHub\)";

const WCHAR* GetRepositoriesRoot()
{
	return repositoriesRoot;
}

void OnShutdown()
{
	g_HTTPServer->close().wait();
}
void OnInitialize(string_t &address)
{
	g_HTTPServer = std::unique_ptr<HTTPServer>(new HTTPServer(address));
	g_HTTPServer->open().wait();
}



int wmain(int argc, wchar_t *argv[])
{
	BOOL result=AddGitInstallationLocationToPath();
	string_t address = L"http://localhost:10240";
	OnInitialize(address);
	std::cout << "Press ENTER to exit." << std::endl;
	std::string line;
	std::getline(std::cin, line);
	OnShutdown();
	return 0;
}