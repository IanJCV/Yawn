#pragma once

#include "common.h"

namespace engine
{

	class ENGINE_API Shader
	{
	public:
		Shader();

		bool Load(const wchar_t* vsFile, const wchar_t* psFile, Device* device);
		bool Load(const char* text, size_t length, const char* name, Device* device);
		void Reload();
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11InputLayout* inputLayout;
		ShaderBlob* vsBlob, * psBlob;

		bool hadError = false;

		static Shader* Find(std::string name);
		
    private:
		const wchar_t* vsName;
		const wchar_t* psName;
		std::string name;
		Device* lastDevice;

		static inline Shader* errorShader = nullptr;
	};

	extern ENGINE_API std::vector<Shader*> allShaders;
}
