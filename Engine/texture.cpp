#include "texture.h"
#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTex/DirectXTex.h>

#include <filesystem>

ENGINE_API std::vector<engine::Texture*> engine::allTextures;

namespace
{
	void LoadTexture(
		Device* device, 
		DeviceContext* ctx,
		ID3D11Texture2D** texture, 
		const char* filename, 
		int& width, int& height, int& channels, 
		D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, 
		UINT BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 
		UINT CPUAccess = 0)
	{
		std::string ext = str2lower(std::filesystem::path(filename).extension().string());

		DirectX::ScratchImage img;
		DirectX::TexMetadata meta;
		std::wstring f = widen(filename);
		const wchar_t* file = f.c_str();

		if (ext == ".dds")
		{
			DirectX::GetMetadataFromDDSFile(file, DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, meta);
			DirectX::LoadFromDDSFile(file, DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, &meta, img);
		}
		else if (ext == ".tga")
		{
			DirectX::GetMetadataFromTGAFile(file, meta);
			DirectX::LoadFromTGAFile(file, 
				DirectX::TGA_FLAGS_ALLOW_ALL_ZERO_ALPHA | DirectX::TGA_FLAGS_FORCE_SRGB,
				&meta,
				img);
		}
		else
		{
			DirectX::GetMetadataFromWICFile(file, DirectX::WIC_FLAGS_FILTER_LINEAR, meta);
			DirectX::LoadFromWICFile(file, DirectX::WIC_FLAGS_FILTER_LINEAR, &meta, img);
		}

		const DirectX::Image* image = img.GetImage(0, 0, 0);

		HRESULT hres = DirectX::CreateTextureEx(device, image, 1, meta, Usage, BindFlags, CPUAccess, 0, DirectX::CREATETEX_DEFAULT, (ID3D11Resource**)texture);

		if (FAILED(hres))
		{
			std::string s(filename);
			DebugOut(L"Failed to load texture at path %s!\n", get_utf16(s));
			assert(false);
		}

		// stbi implementation
		// unsigned char* result;
		// result = stbi_load(filename, &width, &height, &channels, 4);

		//D3D11_TEXTURE2D_DESC desc;
		//desc.Height = height;
		//desc.Width = width;
		//desc.MipLevels = 4;
		//desc.ArraySize = 1;
		//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//desc.SampleDesc.Count = 1;
		//desc.SampleDesc.Quality = 0;
		//desc.Usage = Usage;
		//desc.BindFlags = BindFlags;
		//desc.CPUAccessFlags = CPUAccess;
		//desc.MiscFlags = BindFlags == 40U ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		//D3D11_SUBRESOURCE_DATA subresource;
		//D3D11_SUBRESOURCE_DATA* d;
		//UINT rowPitch = (width * 4) * sizeof(unsigned char);

		//if (Usage == D3D11_USAGE_DEFAULT)
		//{
		//	d = NULL;
		//}
		//else
		//{
		//	d = &subresource;

		//	subresource.pSysMem = (const void*)image->pixels;
		//	subresource.SysMemPitch = image->rowPitch;
		//	subresource.SysMemSlicePitch = 0;
		//}

		//if (Usage == D3D11_USAGE_DEFAULT)
		//{
		//	ctx->UpdateSubresource(*texture, 0, NULL, result, rowPitch, 0);
		//}

		//delete[] result;
	}
}

namespace texture
{

	void Load(const char* filename, DirectX::ScratchImage& image, DirectX::TexMetadata& metadata)
	{
		std::string ext = str2lower(std::filesystem::path(filename).extension().string());

		std::wstring f = widen(filename);
		const wchar_t* file = f.c_str();

		if (ext == ".dds")
		{
			DirectX::GetMetadataFromDDSFile(file, DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, metadata);
			DirectX::LoadFromDDSFile(file, DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, &metadata, image);
		}
		else if (ext == ".tga")
		{
			DirectX::GetMetadataFromTGAFile(file, metadata);
			DirectX::LoadFromTGAFile(file,
				DirectX::TGA_FLAGS_ALLOW_ALL_ZERO_ALPHA | DirectX::TGA_FLAGS_FORCE_SRGB,
				&metadata,
				image);
		}
		else
		{
			DirectX::GetMetadataFromWICFile(file, DirectX::WIC_FLAGS_FILTER_LINEAR, metadata);
			DirectX::LoadFromWICFile(file, DirectX::WIC_FLAGS_FILTER_LINEAR, &metadata, image);
		}
	}

	void Load(const char* filename, Device* device, ID3D11Texture2D** tex, ID3D11ShaderResourceView** view, DirectX::TexMetadata& meta, 

		D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT BindFlags = D3D11_BIND_SHADER_RESOURCE, UINT CpuFlags = 0, UINT MiscFlags = 0)
	{
		std::string ext = str2lower(std::filesystem::path(filename).extension().string());

		std::wstring f = widen(filename);
		const wchar_t* file = f.c_str();

		if (ext == ".dds")
		{
			DirectX::GetMetadataFromDDSFile(file, DirectX::DDS_FLAGS_ALLOW_LARGE_FILES, meta);
			DirectX::CreateDDSTextureFromFileEx(device, file, 0Ui64, usage, BindFlags, CpuFlags, MiscFlags, DirectX::DDS_LOADER_DEFAULT, (ID3D11Resource**)tex, view);
		}
		else if (ext == ".tga")
		{
			DirectX::GetMetadataFromTGAFile(file, meta);
			DirectX::ScratchImage img;
			DirectX::LoadFromTGAFile(file, &meta, img);
			if (view != NULL)
			{
				DirectX::CreateShaderResourceViewEx(device, img.GetImage(0, 0, 0), 1, meta, usage, BindFlags, CpuFlags, MiscFlags, DirectX::CREATETEX_DEFAULT, view);
			}
			if (tex != NULL)
			{
				DirectX::CreateTexture(device, img.GetImage(0, 0, 0), 1, meta, (ID3D11Resource**)tex);
			}
		}
		else
		{
			DirectX::GetMetadataFromWICFile(file, DirectX::WIC_FLAGS_FILTER_LINEAR, meta);
			DirectX::CreateWICTextureFromFile(device, file, (ID3D11Resource**)tex, view);
		}
	}

	int GetPitch(const char* filename)
	{
		using namespace DirectX;
		TexMetadata metadata;
		ScratchImage image;

		Load(filename, image, metadata);

		int pitch = image.GetImage(0, 0, 0)->rowPitch;

		image.Release();
		return pitch;
	}
}

inline engine::Texture::Texture(const char* filename)
	:name(std::filesystem::path(filename).filename().string())
{
	ID3D11Texture2D* texture;

	std::wstring f = widen(filename);
	HRESULT hr = DirectX::CreateWICTextureFromFile(device, f.c_str(), (ID3D11Resource**)&texture, &textureView);

	D3D11_TEXTURE2D_DESC desc;

	texture->GetDesc(&desc);

	this->width = desc.Width;
	this->height = desc.Height;

	texture->Release();

	if (FAILED(hr))
	{
		std::string s(filename);
		DebugOut(L"Failed to load texture at path %s!\n", get_utf16(s));
		assert(false);
	}
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
	if (filenames.size() != 6)
	{
		DebugOut(L"Incorrect number of texture paths passed into cubemap loader! Passed in: %d, needs 6\n", filenames.size());
		assert(false);
	}

	using namespace DirectX;
	std::vector<TexMetadata> metas(6);
	std::vector<ScratchImage> sImages(6);

	Image images[6];

	for (size_t i = 0; i < 6; i++)
	{
		texture::Load(filenames[i].c_str(), sImages[i], metas[i]);

		images[i] = *sImages[i].GetImage(0, 0, 0);
	}

	ScratchImage im;
	im.InitializeCubeFromImages(images, 6);

	CreateShaderResourceView(device, im.GetImages(), im.GetImageCount(), im.GetMetadata(), &shaderResourceView);
}


engine::Texture* engine::Texture::GetTexture(const char* path)
{
	engine::Texture* tex;
	for (size_t i = 0; i < engine::allTextures.size(); i++)
	{
		if (engine::allTextures[i]->name == std::filesystem::path(path).filename().string())
		{
			tex = engine::allTextures[i];
			return tex;
		}
	}

	tex = new engine::Texture(path);
	engine::allTextures.push_back(tex);
	return tex;
}