#pragma once
#include "pch.h"

#if DLLCOMPILE
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

#define PI 3.14159256f
#define DEG2RAD 0.017453292f

#define _deg2rad(deg) (DEG2RAD * deg)

API void DebugOut(const wchar_t* fmt, ...);

API std::wstring get_utf16(const std::string& str);

API constexpr float d2r(float deg);

API float deg2rad(float deg);