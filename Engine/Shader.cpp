#include "Shader.h"

engine::Shader::Shader()
{
    errorBlob       = nullptr;
    vsBlob          = nullptr;
    psBlob          = nullptr;
    inputLayout     = nullptr;
    vertexShader    = nullptr;
    pixelShader     = nullptr;
}

inline bool engine::Shader::Load(const wchar_t* vsFile, const wchar_t* psFile, Device* device)
{
    // Shader stuff
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ShaderBlob* tempBlob = NULL;

    // COMPILE VERTEX SHADER
    HRESULT hr = D3DCompileFromFile(
        vsFile,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vs_main",
        "vs_5_0",
        flags,
        0,
        &tempBlob,
        &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        if (tempBlob) { tempBlob->Release(); }
        return false;
    }

    vsBlob = tempBlob;

    // COMPILE PIXEL SHADER
    hr = D3DCompileFromFile(
        psFile,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "ps_main",
        "ps_5_0",
        flags,
        0,
        &tempBlob,
        &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
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

    if (FAILED(hr))
    {
        return false;
    }

    hr = device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        NULL,
        &pixelShader);

    if (FAILED(hr))
    {
        return false;
    }

    hr = device->CreateInputLayout(
        Vertex::InputElements,
        Vertex::InputElementCount,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &inputLayout);

    if (FAILED(hr))
    {
        return false;
    }

    vsName = vsFile;
    psName = psFile;

    lastDevice = device;

    return true;
}

void engine::Shader::Reload()
{
	if (errorBlob) errorBlob->Release();
	vsBlob->Release();
	psBlob->Release();
	inputLayout->Release();
	vertexShader->Release();
	pixelShader->Release();

    Load(vsName, psName, lastDevice);
}
