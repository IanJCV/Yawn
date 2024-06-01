#pragma once
#include "pch.h"

#if ENGINE_DLLCOMPILE
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#define Device ID3D11Device
#define DeviceContext ID3D11DeviceContext
#define SwapChain IDXGISwapChain
#define RenderTargetView ID3D11RenderTargetView
#define DepthTargetView ID3D11DepthStencilView
#define ShaderBlob ID3DBlob

#define VertexShader ID3D11VertexShader
#define PixelShader ID3D11PixelShader

#define Vertex DirectX::VertexPositionNormalColorTexture

#define PI 3.14159256f
#define DEG2RAD 0.017453292f

#define _deg2rad(deg) (DEG2RAD * deg)

extern ENGINE_API bool g_Vsync;

ENGINE_API void DebugOut(const wchar_t* fmt, ...);

ENGINE_API std::wstring get_utf16(const std::string& str);

ENGINE_API constexpr float d2r(const float& deg);

ENGINE_API constexpr DirectX::SimpleMath::Vector3 d2r(const DirectX::SimpleMath::Vector3& angles);

ENGINE_API float deg2rad(float deg);

ENGINE_API std::string str2lower(std::string s);

ENGINE_API std::wstring widen(const std::string& str);

ENGINE_API std::string narrow(const std::wstring& str);