#pragma once
#include <Windows.h>

#define ZeroMem(s) ZeroMemory(&s, sizeof(s))



#define PASS_SINGLE(a) static_cast<UINT>(sizeof(a)), 1, &a