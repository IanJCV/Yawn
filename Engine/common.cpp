#include "common.h"

#include <iostream>
#include <xlocale>
#include <sstream>

ENGINE_API bool g_Vsync = false;

ENGINE_API void DebugOut(const wchar_t* fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    wchar_t dbg_out[4096];
    vswprintf_s(dbg_out, fmt, argp);
    va_end(argp);
    OutputDebugString(dbg_out);
}

ENGINE_API std::wstring get_utf16(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}

ENGINE_API constexpr float d2r(const float& deg)
{
    return DEG2RAD * deg;
}

ENGINE_API constexpr DirectX::SimpleMath::Vector3 d2r(const DirectX::SimpleMath::Vector3& angles)
{
    return DirectX::SimpleMath::Vector3(d2r(angles.x), d2r(angles.y), d2r(angles.z));
}

ENGINE_API float deg2rad(float deg)
{
    return DEG2RAD * deg;
}

ENGINE_API std::string str2lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

ENGINE_API std::wstring widen(const std::string& str)
{
    std::wostringstream wstm;
    const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc());
    for (size_t i = 0; i < str.size(); ++i)
        wstm << ctfacet.widen(str[i]);
    return wstm.str();
}

ENGINE_API std::string narrow(const std::wstring& str)
{
    std::ostringstream stm;

    const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(stm.getloc());

    for (size_t i = 0; i < str.size(); ++i)
        stm << ctfacet.narrow(str[i], 0);
    return stm.str();
}
