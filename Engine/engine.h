#pragma once
#include "pch.h"
#include "audio.h"
#include "common.h"

#include <DirectXTK/VertexTypes.h>

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

class API AABB
{
public:
    DirectX::SimpleMath::Vector3 min;
    DirectX::SimpleMath::Vector3 max;
};

class API Mesh
{
public:
    ID3D11Buffer* vBuffer;
    ID3D11Buffer* iBuffer;

    UINT vertex_count;
    UINT vertex_stride;
    UINT vertex_offset;

    UINT index_count;

    AABB boundingBox;
};

class API Model
{
public:
    std::vector<Mesh*> meshes;
    uint32_t meshCount;
    ID3D11InputLayout* layout = NULL;

    std::vector<ID3D11Buffer*> vBuffers;
    std::vector<UINT> strides;
    std::vector<UINT> offsets;
    std::vector<ID3D11Buffer*> iBuffers;
};

// D3D Globals
extern API Device* device;
extern API DeviceContext* deviceContext;
extern API SwapChain* swapChain;
extern API RenderTargetView* renderTarget;
extern API DepthTargetView* depthTarget;
extern API ID3D11DepthStencilState* depthState;

extern API ID3D11RasterizerState* rasterizerState;

extern API ShaderBlob* vsBlob;
extern API ShaderBlob* psBlob;
extern API ShaderBlob* error_blob;
extern API VertexShader* vertexShader;
extern API PixelShader* pixelShader;

extern API int global_windowWidth;
extern API int global_windowHeight;

API void DirectXSetup(HWND hwnd);

API bool ReloadShaders();

API void ResizeWindow();

API Model* LoadModel(const char* filename);

API void ClearBackground(DirectX::SimpleMath::Color color);