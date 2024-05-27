#include "engine.h"

#include "filesystem"
#include <DirectXTK/SimpleMath.h>
#include <DirectXTK/CommonStates.h>

using namespace DirectX;
using namespace SimpleMath;

API Device* device;
API DeviceContext* deviceContext;
API SwapChain* swapChain;
API RenderTargetView* renderTarget;
API DepthTargetView* depthTarget;

API ID3D11DepthStencilState* depthState;

API ID3D11RasterizerState* rasterizerState;

API ShaderBlob* vsBlob;
API ShaderBlob* psBlob;
API ShaderBlob* error_blob;

API VertexShader* vertexShader;
API PixelShader* pixelShader;

API int global_windowWidth;
API int global_windowHeight;

API void DirectXSetup(HWND hwnd)
{
    // Init DX11
    DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
    swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_descr.SampleDesc.Count = 1;
    swap_chain_descr.SampleDesc.Quality = 0;
    swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descr.BufferCount = 1;
    swap_chain_descr.OutputWindow = hwnd;
    swap_chain_descr.Windowed = true;

    D3D_FEATURE_LEVEL feature_level;
    D3D_FEATURE_LEVEL feature_levels[] =
    {
        D3D_FEATURE_LEVEL_11_1
    };

    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

    IDXGIFactory1* factory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));

    UINT i = 0;
    IDXGIAdapter1* pAdapter;
    std::vector <IDXGIAdapter1*> vAdapters;
    while (factory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);

        DXGI_ADAPTER_DESC1 desc;
        HRESULT h = pAdapter->GetDesc1(&desc);
        assert(SUCCEEDED(h));
        DebugOut(L"Adapter %d found: name: %s\n", i, desc.Description);

        ++i;
    }

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &swap_chain_descr,
        &swapChain,
        &device,
        &feature_level,
        &deviceContext);

    assert(S_OK == hr && swapChain && device && deviceContext);

    DebugOut(L"feature level: %d\n", feature_level);

    // create a framebuffer
    Texture2D* framebuffer;
    hr = swapChain->GetBuffer(
        0,
        __uuidof(Texture2D),
        (void**)&framebuffer);
    assert(SUCCEEDED(hr));

    hr = device->CreateRenderTargetView(
        framebuffer, 0, &renderTarget);
    assert(SUCCEEDED(hr));
    framebuffer->Release();

    // resize
    D3D11_TEXTURE2D_DESC desc;
    framebuffer->GetDesc(&desc);
    global_windowWidth = desc.Width;
    global_windowHeight = desc.Height;


    // depth
    /*
    D3D11_DEPTH_STENCIL_DESC depthDesc;

    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    depthDesc.StencilEnable = true;
    depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

    depthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

    hr = device->CreateDepthStencilState(&depthDesc, &depthState);

    deviceContext->OMSetDepthStencilState(depthState, 0);

    assert(SUCCEEDED(hr));
    */


    std::unique_ptr<CommonStates> states(new CommonStates(device));

    deviceContext->OMSetBlendState(states->Opaque(), Color(0, 0, 0, 0), 0xFFFFFFFF);
    deviceContext->OMSetDepthStencilState(states->DepthDefault(), 0);

    rasterizerState = states->CullClockwise();
    deviceContext->RSSetState(rasterizerState);

    auto samplerState = states->LinearWrap();
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    D3D11_TEXTURE2D_DESC depthTextureDesc;
    ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
    depthTextureDesc.Width = desc.Width;
    depthTextureDesc.Height = desc.Height;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* DepthStencilTexture;
    hr = device->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);

    assert(SUCCEEDED(hr));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = depthTextureDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    hr = device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &depthTarget);
    DepthStencilTexture->Release();

    assert(SUCCEEDED(hr));


    if (!ReloadShaders())
    {
        DebugOut(L"Initial shader load failed!\n");
        assert(false);
    }
}

API bool ReloadShaders()
{
    // Shader stuff
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ShaderBlob* tempBlob;

    // COMPILE VERTEX SHADER
    HRESULT hr = D3DCompileFromFile(
        L"shaders/shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        flags,
        0,
        &tempBlob,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (tempBlob) { tempBlob->Release(); }
        return false;
    }

    vsBlob = tempBlob;

    // COMPILE PIXEL SHADER
    hr = D3DCompileFromFile(
        L"shaders/shaders.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        flags,
        0,
        &tempBlob,
        &error_blob);
    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (tempBlob) { tempBlob->Release(); }
        return false;
    }

    psBlob = tempBlob;

    hr = device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        NULL,
        &vertexShader);
    assert(SUCCEEDED(hr));

    hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        NULL,
        &pixelShader);
    assert(SUCCEEDED(hr));

    return true;
}

API void ResizeWindow()
{
    deviceContext->OMSetRenderTargets(0, 0, 0);
    renderTarget->Release();
    depthTarget->Release();

    // render target
    HRESULT res = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(res));

    Texture2D* frameBuffer;
    res = swapChain->GetBuffer(0, __uuidof(Texture2D), (void**)&frameBuffer);
    assert(SUCCEEDED(res));

    res = device->CreateRenderTargetView(frameBuffer, NULL, &renderTarget);

    assert(SUCCEEDED(res));

    D3D11_TEXTURE2D_DESC desc;
    frameBuffer->GetDesc(&desc);
    DebugOut(L"framebuffer resized: %dx%d\n", desc.Width, desc.Height);
    global_windowWidth = desc.Width;
    global_windowHeight = desc.Height;

    frameBuffer->Release();

    // depth
    D3D11_TEXTURE2D_DESC depthTextureDesc;
    ZeroMemory(&depthTextureDesc, sizeof(depthTextureDesc));
    depthTextureDesc.Width = desc.Width;
    depthTextureDesc.Height = desc.Height;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* DepthStencilTexture;
    res = device->CreateTexture2D(&depthTextureDesc, NULL, &DepthStencilTexture);

    assert(SUCCEEDED(res));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = depthTextureDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    res = device->CreateDepthStencilView(DepthStencilTexture, &dsvDesc, &depthTarget);
    
    assert(SUCCEEDED(res));

    DepthStencilTexture->Release();
}

API void ClearBackground(DirectX::SimpleMath::Color color)
{
    deviceContext->ClearRenderTargetView(renderTarget, color);
    deviceContext->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

API Model* LoadModel(const char* filename)
{
    if (!std::filesystem::exists(filename))
    {
        return nullptr;
    }

    Model* out;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenBoundingBoxes);

    if (scene == nullptr)
    {
        return nullptr;
    }

    out = new Model();

    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* rMesh = scene->mMeshes[i];
        Mesh* tMesh = new Mesh();

        tMesh->index_count = rMesh->mNumFaces * 3;

        tMesh->vertex_count = rMesh->mNumVertices;
        tMesh->vertex_stride = sizeof(Vertex);
        tMesh->vertex_offset = 0;

        std::vector<Vertex> verts;

        for (size_t v = 0; v < rMesh->mNumVertices; v++)
        {
            aiVector3D pos = rMesh->mVertices[v];
            aiVector3D norm = rMesh->mNormals[v];

            aiColor4D color = aiColor4D(0.f, 0.f, 0.f, 0.f);
            if (rMesh->HasVertexColors(0))
            {
				color = rMesh->mColors[0][v];
            }

            aiVector3D texcoord = rMesh->mTextureCoords[0][v];

            verts.push_back(Vertex(Vector3(pos.x, pos.y, pos.z), Vector3(norm.x, norm.y, norm.z), Color(color.r, color.g, color.b, color.a), Vector2(texcoord.x, texcoord.y)));
        }

        uint32_t* indices = new uint32_t[rMesh->mNumFaces * 3];

        AABB aabb;
        auto min = rMesh->mAABB.mMin;
        aabb.min = Vector3(min.x, min.y, min.z);

        auto max = rMesh->mAABB.mMax;
        aabb.max = Vector3(max.x, max.y, max.z);

        tMesh->boundingBox = aabb;

        for (size_t f = 0; f < rMesh->mNumFaces; f++)
        {
            aiFace face = rMesh->mFaces[f];
            for (size_t fi = 0; fi < face.mNumIndices; fi++)
            {
                indices[(f * 3) + fi] = face.mIndices[fi];
            }
        }

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = verts.size() * sizeof(Vertex);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA subresourceData = { verts.data()};


        HRESULT hResult = device->CreateBuffer(&vertexBufferDesc, &subresourceData, &tMesh->vBuffer);
        assert(SUCCEEDED(hResult));


        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = tMesh->index_count * sizeof(uint32_t);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices };

        hResult = device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &tMesh->iBuffer);
        assert(SUCCEEDED(hResult));

        out->meshes.push_back(tMesh);
        out->meshCount++;

        out->vBuffers.push_back(tMesh->vBuffer);
        out->iBuffers.push_back(tMesh->iBuffer);
        out->strides.push_back(tMesh->vertex_stride);
        out->offsets.push_back(tMesh->vertex_offset);
    }

    return out;
}
