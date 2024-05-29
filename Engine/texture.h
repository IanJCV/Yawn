#pragma once
#include "common.h"
#include "globals.h"

namespace engine
{

	class ENGINE_API Texture
	{
	public:
		Texture(const char* filename);

		~Texture();

		int width, height;
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* textureView;
	private:

	};

	class ENGINE_API CubemapTexture
	{
	public:
		CubemapTexture(std::vector<std::string> filenames);

		ID3D11Texture2D* cubeTexture = NULL;
		ID3D11ShaderResourceView* shaderResourceView = NULL;

		int width, height;
	};

}