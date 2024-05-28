#pragma once

#include "common.h"

namespace engine
{
	class API Shader
	{
	public:
		Shader();

		bool Load(const wchar_t* vsFile, const wchar_t* psFile, Device* device);
    private:
        ShaderBlob *vsBlob, *psBlob, *errorBlob;
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
		ID3D11InputLayout* inputLayout;
	};
}
