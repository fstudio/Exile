//
//
#include "Precompiled.h"
#include <cpprest/json.h>
using namespace utility;

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsRunOnWin64()
{
	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process) {
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
			//handle error
		}
	}
	return bIsWow64;
}

BOOL WINAPI AppendPathToEnvironment(const std::wstring &path)
{
	WCHAR buffer[32767];
	wcscpy_s(buffer, path.c_str());
	wcscat_s(buffer, L";");
	auto size = GetEnvironmentVariableW(L"PATH", buffer + path.size() + 1, 32766 - path.size());
	if (size <= 0)
		return FALSE;
	SetEnvironmentVariableW(L"PATH", buffer);
	return TRUE;
}

BOOL WINAPI FindGitInstallationLocation(std::wstring &location)
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1 InstallLocation
	HKEY hInst = nullptr;
	LSTATUS result = ERROR_SUCCESS;
	const wchar_t *git4win = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1)";
	const wchar_t *installKey = L"InstallLocation";
	WCHAR buffer[4096] = { 0 };
#if defined(_M_X64)
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ, &hInst) != ERROR_SUCCESS) {
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ|KEY_WOW64_32KEY, &hInst) != ERROR_SUCCESS) {
			//Cannot found msysgit or Git for Windows install
			return FALSE;
		}
	}
#else
	if (IsRunOnWin64()) {
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ | KEY_WOW64_64KEY, &hInst) != ERROR_SUCCESS) {
			if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ , &hInst) != ERROR_SUCCESS) {
				//Cannot found msysgit or Git for Windows install
				return FALSE;
			}
		}
	} else {
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ, &hInst) != ERROR_SUCCESS) {
			return FALSE;
		}
	}
#endif
	DWORD type = 0;
	DWORD dwSize = 4096 * sizeof(wchar_t);
	result = RegGetValueW(hInst, nullptr, installKey, RRF_RT_REG_SZ, &type, buffer, &dwSize);
	if (result == ERROR_SUCCESS) {
		location.assign(buffer);
	}
	RegCloseKey(hInst);
	return result == ERROR_SUCCESS;
}


BOOL WINAPI AddGitInstallationLocationToPath()
{
	std::wstring location;
	if (FindGitInstallationLocation(location)) {
		if (location.back() == L'\\' || location.back() == '/')
			location.append(L"bin");
		else
			location.append(L"\\bin");
		return AppendPathToEnvironment(location);
	}
	return FALSE;
}

LRESULT WINAPI ParseParameters(LPCWSTR lpProfile)
{
	//
	return S_OK;
}