#pragma once
#include "pch.h"

#if DLLCOMPILE
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

#define Device ID3D11Device
#define DeviceContext ID3D11DeviceContext
#define SwapChain IDXGISwapChain
#define Texture2D ID3D11Texture2D
#define RenderTargetView ID3D11RenderTargetView
#define DepthTargetView ID3D11DepthStencilView
#define ShaderBlob ID3DBlob

#define VertexShader ID3D11VertexShader
#define PixelShader ID3D11PixelShader

#define Vertex DirectX::VertexPositionNormalColorTexture

#define PI 3.14159256f
#define DEG2RAD 0.017453292f

#define _deg2rad(deg) (DEG2RAD * deg)

API void DebugOut(const wchar_t* fmt, ...);

API std::wstring get_utf16(const std::string& str);

API constexpr float d2r(float deg);

API float deg2rad(float deg);