//

#ifndef PRECOMPILED_H
#define PRECOMPILED_H
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

enum DebugMessageLevel {
	kDebug,
	kInfo,
	kWarn,
	kError,
	kFatal
};

#ifdef _DEBUG
//template<typename... Args>
//int DebugMessage(const wchar_t *format, Args... args);
int DebugMessage(DebugMessageLevel level, const wchar_t *format, ...);
#else
#define DebugMessage noop
#endif

BOOL WINAPI AppendPathToEnvironment(const std::wstring &path);
BOOL WINAPI FindGitInstallationLocation(std::wstring &location);
BOOL WINAPI AddGitInstallationLocationToPath();

#endif
