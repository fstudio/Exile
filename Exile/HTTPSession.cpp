//
#include "Precompiled.h"
#include <PathCch.h>
#include <array>
///
#include "GZipStream.h"
#include "HTTPSession.h"
#include "ProfileMetadata.h"

extern ProfileMetadata g_profileMetadata;

static BOOL GitRepositoryAccessCheck(const std::wstring &path) {
  WIN32_FILE_ATTRIBUTE_DATA attr_data;
  if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr_data)) {
    if (attr_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return TRUE;
  }
  return FALSE;
}

bool HTTPSession::Execute(const std::wstring &relativePath, int channel) {
  const CHAR *context_type = nullptr;
  std::vector<wchar_t> cmdx(PATHCCH_MAX_CCH);
  WCHAR cmdline[PATHCCH_MAX_CCH] = L"git ";
  // concurrency::streams::streambuf<uint8_t> respbuf;
  utf8string body;
  auto &header = response_.headers();
  header.add(L"Expires", L"Fri, 01 Jan 1980 00:00:00 GMT");
  header.add(L"Pragma", L"no-cache");
  header.add(L"Cache-Control", L"no-cache, max-age=0, must-revalidate");
  uint8_t uploadpack_header[] = "001e# service=git-upload-pack\n0000";
  uint8_t receivepack_header[] = "001f# service=git-receive-pack\n0000";
  switch (channel) {
  case kGitUploadPackLs:
    wcscat_s(cmdline, L" upload-pack --stateless-rpc --advertise-refs .");
    context_type = "application/x-git-upload-pack-advertisement";
    body.append((char *)uploadpack_header, sizeof(uploadpack_header) - 1);
    break;
  case kGitUploadPackStream:
    wcscat_s(cmdline, L" upload-pack --stateless-rpc .");
    context_type = "application/x-git-upload-pack-result";
    break;
  case kGitReceivePackLs:
    wcscat_s(cmdline, L" receive-pack --stateless-rpc --advertise-refs .");
    context_type = "application/x-git-receive-pack-advertisement";
    body.append((char *)receivepack_header, sizeof(receivepack_header) - 1);
    break;
  case kGitReceivePackStream:
    wcscat_s(cmdline, L" receive-pack --stateless-rpc .");
    context_type = "application/x-git-receive-pack-result";
    break;
  default:
    return false;
  }
  std::wstring repopath = g_profileMetadata.Root();
  if (repopath.back() == '\\')
    repopath.pop_back();
  repopath.append(relativePath);
  if (!GitRepositoryAccessCheck(repopath)) {
    request_.reply(status_codes::NotFound, L"Not Found Repository !");
    return false;
  }
  HANDLE hPipeOutputRead = NULL;
  HANDLE hPipeOutputWrite = NULL;
  HANDLE hPipeInputRead = NULL;
  HANDLE hPipeInputWrite = NULL;

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;
  CreatePipe(&hPipeOutputRead, &hPipeOutputWrite, &sa, PIPE_BUFFER_SIZE);
  CreatePipe(&hPipeInputRead, &hPipeInputWrite, &sa, PIPE_BUFFER_SIZE);
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  GetStartupInfoW(&si);
  si.dwFlags = STARTF_USESHOWWINDOW |
               STARTF_USESTDHANDLES; // use hStdInput hStdOutput hStdError
  si.wShowWindow = SW_HIDE;
  si.hStdInput = hPipeInputRead;
  si.hStdOutput = hPipeOutputWrite;
  si.hStdError = hPipeOutputWrite;
  /// CreateProcess bInheritHandles require TRUE
  if (!CreateProcessW(NULL, cmdline, NULL, NULL, TRUE, 0, NULL,
                      repopath.c_str(), &si, &pi)) {
    CloseHandle(hPipeOutputRead);
    CloseHandle(hPipeInputWrite);
    CloseHandle(hPipeOutputWrite);
    CloseHandle(hPipeInputRead);
    request_.reply(status_codes::InternalError, L"Internal Error");
    return false;
  }
  CloseHandle(pi.hThread);
  CloseHandle(hPipeOutputWrite);
  CloseHandle(hPipeInputRead);
  if (channel == kGitUploadPackStream || channel == kGitReceivePackStream) {
    auto &h = request_.headers();
    //(L"Content-Encoding", L"gzip")
    bool encodingGzip = false;
    if (h.has(L"Content-Encoding") &&
        h[L"Content-Encoding"].compare(L"gzip") == 0) {
      encodingGzip = true;
    }
    auto &stream = request_.body();
    concurrency::streams::container_buffer<std::string> inStringBuffer;
    stream.read_to_end(inStringBuffer)
        .then(
            [inStringBuffer, hPipeInputWrite, encodingGzip](size_t bytesRead) {
              DWORD dwWrite = 0;
              const std::string &buffer = inStringBuffer.collection();
              if (encodingGzip) {
                GZipStreamWritePipe(hPipeInputWrite, (uint8_t *)buffer.data(),
                                    bytesRead);
              } else {
                WriteFile(hPipeInputWrite, buffer.data(), bytesRead, &dwWrite,
                          NULL);
              }
              // std::cout.write(inStringBuffer.collection().data(), bytesRead);
            })
        .get();

    // FIXME
  }
  // concurrency::streams::ostream ostream;
  BOOL bSuccess;
  uint8_t buffer[PIPE_BUFFER_SIZE] = {0};
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
    // ostream.write(Concurrency::streams::streambuf<uint8_t>(buffer.),
    // dwNumberOfBytesRead);
    body.append((char *)buffer, dwNumberOfBytesRead);
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  response_.set_body(body, context_type);
  // Close all remaining handles
  CloseHandle(pi.hProcess);
  CloseHandle(hPipeOutputRead);
  CloseHandle(hPipeInputWrite);
  response_.set_status_code(status_codes::OK);
  request_.reply(response_);
  // request_.reply(status_codes::OK).then([](){
  // //do some thing
  //});
  return true;
}
