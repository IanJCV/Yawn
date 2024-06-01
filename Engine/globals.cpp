#include "globals.h"

namespace engine
{
	ENGINE_API int g_WindowWidth = 0;
	ENGINE_API int g_WindowHeight = 0;

	ENGINE_API DeviceContext* immediateContext;
	ENGINE_API Device* device;
	ENGINE_API ID3D11SamplerState* defaultSamplerState;

	ENGINE_API ID3D11RasterizerState* rasterizerState;
	ENGINE_API ID3D11RasterizerState* wireframeState;
}