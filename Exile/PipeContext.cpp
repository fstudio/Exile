//
//
//
//
#include "PipeContext.h"
#include <chrono>
#include <atomic>
namespace {
	std::atomic<uint32_t> g_pipe_serial(0);
}
class PipeWindows {
public:
	PipeWindows()
	{
		m_read = INVALID_HANDLE_VALUE;
		m_write = INVALID_HANDLE_VALUE;
		ZeroMemory(&m_read_overlapped, sizeof(m_read_overlapped));
		ZeroMemory(&m_write_overlapped, sizeof(m_write_overlapped));
	}
	~PipeWindows()
	{
		Close();
	}
	HRESULT Create(bool child_process_inherit)
	{
		WCHAR name[MAX_PATH];
		uint32_t serial = g_pipe_serial.fetch_add(1);
		swprintf_s(name, L"Exile.Pipe.%d.%d", GetCurrentProcessId(), serial);
		return Create(name,child_process_inherit);
	}
	HRESULT Create(const std::wstring &name, bool child_process_inherit)
	{
		if (name.empty())
			return ERROR_INVALID_PARAMETER;
		if (CanRead() || CanWrite()) {
			return ERROR_ALREADY_EXISTS;
		}
		std::wstring pipename = LR"(\\.\Pipe\)";
		DWORD read_mode = FILE_FLAG_OVERLAPPED;
		m_read = ::CreateNamedPipeW(pipename.c_str(), PIPE_ACCESS_INBOUND | read_mode, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 1024,
									1024, 120 * 1000, NULL);
		if (INVALID_HANDLE_VALUE == m_read) {
			return S_FALSE;
		}
		ZeroMemory(&m_read_overlapped, sizeof(m_read_overlapped));
		m_read_overlapped.hEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

		// Open the write end of the pipe.
		//Error result = OpenNamedPipe(name, child_process_inherit, false);
		return S_OK;
	}
	HRESULT OpenNamedPipe(std::wstring name, bool child_process_inherit, bool is_read)
	{
		if (name.empty())
			return S_FALSE;

		return S_OK;
	}
	bool CanRead()const
	{
		return (m_read != INVALID_HANDLE_VALUE);
	}
	bool CanWrite()const
	{
		return(m_write != INVALID_HANDLE_VALUE);
	}
	void Close()
	{
		/////
	}
private:
	HANDLE m_read;
	HANDLE m_write;
	OVERLAPPED m_read_overlapped;
	OVERLAPPED m_write_overlapped;
};

PipeContext::PipeContext()
{
	//
}