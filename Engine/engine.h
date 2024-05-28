#pragma once
#include "pch.h"
#include "audio.h"
#include "common.h"
#include "globals.h"

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

API void DirectXSetup(HWND hwnd);

API bool ReloadShaders();

API void ResizeWindow();

API Model* LoadModel(const char* filename);

API void ClearBackground(DirectX::SimpleMath::Color color);