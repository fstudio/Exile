///
///
///
#include "Precompiled.h"
//
#include <PathCch.h>
#include <memory>
#include "HTTPServer.h"
#include "HTTPSession.h"
#include "ExileSettings.h"

extern ExileSettings exileSettings;

static BOOL RepositoryIsExists(const std::wstring &path)
{
	WIN32_FILE_ATTRIBUTE_DATA attr_data;
	if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr_data)) {
		if (attr_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;
	}
	return FALSE;
}

static void ResolveRepositoryPath(std::wstring &repopath, const std::wstring &relativePath)
{
	repopath.assign(exileSettings.Root());
	if (repopath.back() == '\\')
		repopath.pop_back();
	repopath.append(relativePath);
}

int HTTPRefsRestore(http_request &message, const std::wstring &relativePath, int channel)
{
	std::wstring repopath;
	ResolveRepositoryPath(repopath, relativePath);
	if (!RepositoryIsExists(repopath)) {
		message.reply(status_codes::NotFound, L"Not Found Repository !");
		return 1;
	}
	HANDLE hPipeStdout, hPipe;
	WCHAR cmdline[PATHCCH_MAX_CCH] = L"git ";
	http_response resp;
	utf8string refs;
	auto &header = resp.headers();
	const CHAR *context_type = nullptr;
	uint8_t uploadpack_header[] = "001e# service=git-upload-pack\n0000";
	uint8_t receivepack_header[] = "001f# service=git-receive-pack\n0000";
	header.add(L"Expires", L"Fri, 01 Jan 1980 00:00:00 GMT");
	header.add(L"Pragma", L"no-cache");
	header.add(L"Cache-Control", L"no-cache, max-age=0, must-revalidate");
	if (channel == kGitUploadPackLs) {
		wcscat_s(cmdline, L" upload-pack --stateless-rpc --advertise-refs .");
		context_type = "application/x-git-upload-pack-advertisement";
		refs.append((char *)uploadpack_header, sizeof(uploadpack_header) - 1);;
	} else if (channel == kGitReceivePackLs) {
		wcscat_s(cmdline, L" receive-pack --stateless-rpc --advertise-refs .");
		context_type = "application/x-git-receive-pack-advertisement";
		refs.append((char *)receivepack_header, sizeof(receivepack_header) - 1);
	} else {
		message.reply(status_codes::NotFound, L"Not Found Repository !");
		return 1;
	}
	HANDLE hPipeOutputRead = NULL;
	HANDLE hPipeOutputWrite = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	CreatePipe(&hPipeOutputRead, &hPipeOutputWrite, &sa, PIPE_BUFFER_SIZE);
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	GetStartupInfoW(&si);
	si.dwFlags = STARTF_USESHOWWINDOW |
		STARTF_USESTDHANDLES; // use hStdInput hStdOutput hStdError
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hPipeOutputWrite;
	si.hStdError = hPipeOutputWrite;
	/// CreateProcess bInheritHandles require TRUE
	if (!CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0, NULL,
		repopath.c_str(), &si, &pi)) {
		CloseHandle(hPipeOutputRead);
		CloseHandle(hPipeOutputWrite);
		message.reply(status_codes::InternalError, L"Start git command failed !");
		return 1;
	}
	CloseHandle(hPipeOutputWrite);
	BOOL bSuccess;
	uint8_t buffer[PIPE_BUFFER_SIZE] = { 0 };
	DWORD dwNumberOfBytesRead;
	while (TRUE) {
		bSuccess = ReadFile(hPipeOutputRead, // handle of the read end of our pipe
							buffer,          // address of buffer that receives data
							PIPE_BUFFER_SIZE,     // number of bytes to read
							&dwNumberOfBytesRead, // address of number of bytes read
							NULL                  // non-overlapped.
							);
		if (!bSuccess) {
			break;
		}
		refs.append((char *)buffer, dwNumberOfBytesRead);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	resp.set_body(refs, context_type);
	CloseHandle(pi.hProcess);
	CloseHandle(hPipeOutputRead);
	resp.set_status_code(status_codes::OK);
	message.reply(resp);
	return 0;
}

int HTTPFetchActive(http_request &message, const std::wstring &relativePath)
{
	/// INN git stdin small stdout big
	WCHAR file_[MAX_PATH];
	wsprintfW(file_, L"%s\\Exile.Temporary.Fetch.%d.packs", exileSettings.Temporary(), GetCurrentThreadId());
	return 0;
}

int HTTPPushActive(http_request &message, const std::wstring &relativePath)
{
	// git stdin big stdout small
	WCHAR file_[MAX_PATH];
	wsprintfW(file_, L"%s\\Exile.Temporary.Push.%d.packs", exileSettings.Temporary(), GetCurrentThreadId());
	return 0;
}