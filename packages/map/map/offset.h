
#pragma once

#include <windows.h>

void DefineOffsets();
DWORD GetDllOffset(int num);
DWORD GetDllOffset(char *DllName, int Offset);
