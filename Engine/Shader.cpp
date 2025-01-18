#include "Shader.h"
#include "errorshader.h"
#include <filesystem>

namespace engine
{
    ENGINE_API std::vector<Shader*> allShaders;

    engine::Shader::Shader()
    {
        vsBlob = nullptr;
        psBlob = nullptr;
        inputLayout = nullptr;
        vertexShader = nullptr;
        pixelShader = nullptr;
    }

    inline bool engine::Shader::Load(const char* text, size_t length, const char* name, Device* device)
    {
        // Shader stuff
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

        ShaderBlob* tempBlob = NULL;
        ShaderBlob* errorBlob = NULL;

        // COMPILE VERTEX SHADER
        HRESULT hr = D3DCompile(
            text, 
            length, 
            "shaders/error.hlsl", 
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
        hr = D3DCompile(
            text,
            length,
            "shaders/error.hlsl",
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

        vsName = widen(name).c_str();
        psName = widen(name).c_str();
        this->name = name;

        allShaders.push_back(this);

        lastDevice = device;

        return true;
    }

    inline Shader* LoadFromCompiled(const char* file, Device* device)
    {
        
    }

    inline bool engine::Shader::Load(const wchar_t* vsFile, const wchar_t* psFile, Device* device)
    {
        if (!errorShader)
        {
            errorShader = new Shader();
            errorShader->Load(errorShaderText, errorShaderTextCount, "error", device);
        }

        vsName = vsFile;
        psName = psFile;
        auto filename = std::filesystem::path(vsFile).filename().string();
        auto extPos = filename.find_last_of('.');
        if (extPos != std::string::npos)
        {
            filename = filename.substr(0, extPos);
        }
        name = filename;
        allShaders.push_back(this);

        lastDevice = device;

        bool success = true;

        ShaderBlob* errorBlob = nullptr;

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

            success = false;
            goto err;
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

            success = false;
            goto err;
        }

        psBlob = tempBlob;

        hr = device->CreateVertexShader(
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            NULL,
            &vertexShader);

        if (FAILED(hr))
        {
            success = false;
            goto err;
        }

        hr = device->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            NULL,
            &pixelShader);

        if (FAILED(hr))
        {
            success = false;
            goto err;
        }

        hr = device->CreateInputLayout(
            Vertex::InputElements,
            Vertex::InputElementCount,
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            &inputLayout);

        if (FAILED(hr))
        {
            success = false;
            goto err;
        }

        return success;
    err:
        vsBlob = errorShader->vsBlob;
        psBlob = errorShader->psBlob;

        vertexShader = errorShader->vertexShader;
        pixelShader = errorShader->pixelShader;

        inputLayout = errorShader->inputLayout;

        return success;
    }

    void engine::Shader::Reload()
    {
        vsBlob->Release();
        psBlob->Release();
        inputLayout->Release();
        vertexShader->Release();
        pixelShader->Release();

        Load(vsName, psName, lastDevice);
    }

    Shader* engine::Shader::Find(std::string name)
    {
        for (size_t i = 0; i < allShaders.size(); i++)
        {
            if (allShaders[i]->name == name)
            {
                return allShaders[i];
            }
        }
        return nullptr;
    }

}