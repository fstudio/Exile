///
///
///
///
///
#ifndef PIPE_CONTEXT_H
#define PIPE_CONTEXT_H
#include <Windows.h>
#include <cstdint>
#include <string>

class PipeContext {
public:
	PipeContext();
	int Exec(const std::wstring &command);
	std::size_t Write(const void *data, size_t bytes);
	std::size_t Read(void *data, size_t bufsize);
	void WaitForExit();
private:
	HANDLE hStandardError;
	HANDLE hStandardOutput;
	HANDLE hStandardInput;
};


#endif
