#pragma once
#include "common.h"

namespace engine
{
	extern ENGINE_API int g_WindowWidth;
	extern ENGINE_API int g_WindowHeight;

	extern ENGINE_API DeviceContext* immediateContext;
	extern ENGINE_API Device* device;

	extern ENGINE_API ID3D11RasterizerState* rasterizerState;
	extern ENGINE_API ID3D11RasterizerState* wireframeState;

	extern ENGINE_API ID3D11SamplerState* defaultSamplerState;
}