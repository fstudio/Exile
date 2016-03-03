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

BOOL WINAPI AppendPathToEnvironment(const std::wstring &path);
BOOL WINAPI FindGitInstallationLocation(std::wstring &location);
BOOL WINAPI AddGitInstallationLocationToPath();

#endif
