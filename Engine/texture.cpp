#include "texture.h"
#include <DirectXTK/WICTextureLoader.h>

inline engine::Texture::Texture(const char* filename)
{
	int channels = 0;
	unsigned char* result = stbi_load(filename, &width, &height, &channels, 4);

	D3D11_TEXTURE2D_DESC desc;
	desc.Height = height;
	desc.Width = width;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	HRESULT hr = device->CreateTexture2D(&desc, NULL, &texture);
	if (FAILED(hr))
	{
		std::string s(filename);
		DebugOut(L"Failed to load texture at path %s!\n", get_utf16(s));
		assert(false);
	}

	unsigned int rowPitch = (width * 4) * sizeof(unsigned char);

	immediateContext->UpdateSubresource(texture, 0, NULL, result, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC rDesc;
	rDesc.Format = desc.Format;
	rDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	rDesc.Texture2D.MostDetailedMip = 0;
	rDesc.Texture2D.MipLevels = -1;

	hr = device->CreateShaderResourceView(texture, &rDesc, &textureView);
	if (FAILED(hr))
	{
		std::string s(filename);
		DebugOut(L"Failed to load texture at path %s!\n", get_utf16(s));
		assert(false);
	}

	immediateContext->GenerateMips(textureView);

	delete[] result;
	result = 0;
}

engine::Texture::~Texture()
{
	if (textureView)
	{
		textureView->Release();
		textureView = 0;
	}

	if (texture)
	{
		texture->Release();
		texture = 0;
	}
}

engine::CubemapTexture::CubemapTexture(std::vector<std::string> filenames)
{
    //Description of each face
    D3D11_TEXTURE2D_DESC texDesc = {};

    D3D11_TEXTURE2D_DESC texDesc1 = {};
    //The Shader Resource view description
    D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc = {};

    ID3D11Texture2D* tex[6] = { nullptr, nullptr, nullptr,nullptr, nullptr, nullptr };

	int channels;

	texDesc1.MipLevels = 0;
	texDesc1.ArraySize = 1;
	texDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc1.SampleDesc.Count = 1;
	texDesc1.SampleDesc.Quality = 0;
	texDesc1.Usage = D3D11_USAGE_DEFAULT;
	texDesc1.BindFlags = 0;
	texDesc1.CPUAccessFlags = 0;
	texDesc1.MiscFlags = 0;

    for (int i = 0; i < 6; i++)
    {
		unsigned char* result = stbi_load(filenames[i].c_str(), &width, &height, &channels, 4);
		texDesc1.Width = width;
		texDesc1.Height = height;

		HRESULT hr = device->CreateTexture2D(&texDesc1, NULL, &tex[i]);
		if (FAILED(hr))
		{
			std::string s(filenames[i]);
			DebugOut(L"Failed to load texture at path '%s'!\n", get_utf16(s).c_str());
			assert(false);
		}
    }

    tex[0]->GetDesc(&texDesc1);

    texDesc.Width = texDesc1.Width;
    texDesc.Height = texDesc1.Height;
    texDesc.MipLevels = texDesc1.MipLevels;
    texDesc.ArraySize = 6;
    texDesc.Format = texDesc1.Format;
    texDesc.CPUAccessFlags = 0;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    SMViewDesc.Format = texDesc.Format;
    SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
    SMViewDesc.TextureCube.MostDetailedMip = 0;

    HRESULT hr = device->CreateTexture2D(&texDesc, NULL, &cubeTexture);

	assert(SUCCEEDED(hr));

	/*
    for (int i = 0; i < 6; i++)
    {

        for (UINT mipLevel = 0; mipLevel < texDesc.MipLevels; ++mipLevel)
        {
            D3D11_MAPPED_SUBRESOURCE mappedTex2D;
            HRESULT hr = (immediateContext->Map(tex[i], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));
            assert(SUCCEEDED(hr));
            immediateContext->UpdateSubresource(cubeTexture,
                D3D11CalcSubresource(mipLevel, i, texDesc.MipLevels),
                0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

            immediateContext->Unmap(tex[i], mipLevel);
        }
    }
	*/

    for (int i = 0; i < 6; i++)
    {
        tex[i]->Release();
    }

    device->CreateShaderResourceView(cubeTexture, &SMViewDesc, &shaderResourceView);
}
