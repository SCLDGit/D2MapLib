
#pragma once

#include <windows.h>

void DefineOffsets();
DWORD GetDllOffset(int num);
DWORD GetDllOffset(const char *DllName, int Offset);
