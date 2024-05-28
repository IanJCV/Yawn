#pragma once

#include "common.h"

namespace engine
{
	class ENGINE_API Shader
	{
	public:
		Shader();

		bool Load(const wchar_t* vsFile, const wchar_t* psFile, Device* device);
		void Reload();
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11InputLayout* inputLayout;
		ShaderBlob* vsBlob, * psBlob, * errorBlob;
		
    private:
		const wchar_t* vsName;
		const wchar_t* psName;
		Device* lastDevice;
	};
}
