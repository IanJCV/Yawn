#include "rendering.h"
#include <filesystem>
#include <DirectXTK/CommonStates.h>

namespace engine
{
	ENGINE_API Renderer& Renderer::Get()
	{
		static Renderer rend;
		return rend;
	}

	ENGINE_API void Renderer::SetContext(DeviceContext** context)
	{
		this->context = context;
	}

	ENGINE_API void Renderer::SetCamera(Camera* camera)
	{
		this->camera = camera;
	}

    ENGINE_API void Renderer::PreUpdate()
    {
        m_ViewProjection = camera->View * camera->Projection;
    }

    ENGINE_API void Renderer::DrawSkybox(CubemapTexture* texture)
    {
        if (!texture)
        {
            DebugOut(L"Invalid skybox texture passed in!\n");
            return;
        }

        auto ctx = (*context);

        ctx->VSSetShader(m_SkyboxShader->vertexShader, NULL, 0);
        ctx->PSSetShader(m_SkyboxShader->pixelShader, NULL, 0);

        ctx->PSSetShaderResources(0, 1, &texture->shaderResourceView);

        void* c = nullptr;
        m_MatrixBuffer.BeginUpdate(ctx, &c);

        MatrixBuffer* buf = (MatrixBuffer*)c;

        Matrix proj;
        Matrix v;
        Matrix m;
        camera->Projection.Transpose(proj);
        camera->View.Transpose(v);
        Matrix::Identity.Transpose(m);

        buf->model = m;
        buf->view = camera->View;
        buf->projection = camera->Projection;
        buf->mvp = m * camera->View * camera->Projection;
        buf->color = Color(1.f, 1.f, 1.f, 1.f);
        buf->camera = camera->Position;
        buf->cameraView = camera->Forward;
        buf->pad = Vector2();

        m_MatrixBuffer.EndUpdate(ctx);

        ctx->IASetInputLayout(m_SkyboxInputLayout);

        m_MatrixBuffer.Use(ctx);

        ctx->IASetVertexBuffers(0, 1, &m_SkyboxMesh->vBuffer, &m_SkyboxMesh->vertex_stride, &m_SkyboxMesh->vertex_offset);
        ctx->IASetIndexBuffer(m_SkyboxMesh->iBuffer, DXGI_FORMAT_R32_UINT, 0);
        ctx->DrawIndexed(m_SkyboxMesh->index_count, 0, 0);
    }

	ENGINE_API void Renderer::SubmitForRendering(Model* model, bool wireframe)
	{
		auto ctx = (*context);

        //if (wireframe)
        //{
        //    ctx->RSSetState(wireframe ? wireframeState : rasterizerState);
        //}
		
		ctx->VSSetShader(model->shader->vertexShader, NULL, 0);
		ctx->PSSetShader(model->shader->pixelShader, NULL, 0);

        ctx->PSSetSamplers(0, 1, &defaultSamplerState);

        void* c = nullptr;
        m_MatrixBuffer.BeginUpdate(ctx, &c);

        MatrixBuffer* buf = (MatrixBuffer*)c;

        Matrix proj;
        Matrix v;
        camera->Projection.Transpose(proj);
        camera->View.Transpose(v);

        buf->model = model->modelMatrix;
        buf->view = v;
        buf->projection = proj;
        buf->mvp = model->modelMatrix * camera->View * camera->Projection;
        buf->color = Color(1.f, 1.f, 1.f, 1.f);
        buf->camera = camera->Position;
        buf->cameraView = camera->Forward;
        buf->pad = Vector2();

        m_MatrixBuffer.EndUpdate(ctx);

		ctx->IASetInputLayout(model->layout);

        m_MatrixBuffer.Use(ctx);

        for (size_t i = 0; i < model->meshCount; i++)
		{
            Mesh* mesh = model->meshes[i];

            if (mesh->texture)
            {
                ctx->PSSetShaderResources(0, 1, &mesh->texture->textureView);
            }

			ctx->IASetVertexBuffers(0, 1, &mesh->vBuffer, &mesh->vertex_stride, &mesh->vertex_offset);

			ctx->IASetIndexBuffer(mesh->iBuffer, DXGI_FORMAT_R32_UINT, 0);

			ctx->DrawIndexed(mesh->index_count, 0, 0);
        }

        //if (wireframe)
        //{
        //    immediateContext->RSSetState(rasterizerState);
        //}
	}

    void ComputeBox(std::vector<DirectX::VertexPosition>& vertices, std::vector<uint32_t>& indices)
    {
        vertices.clear();
        indices.clear();

        using namespace DirectX;

        indices =
        {
            0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1
        };

        vertices =
        {
            VertexPosition(Vector3(-1, -1, -1)),
            VertexPosition(Vector3(1, -1, -1)),
            VertexPosition(Vector3(1, 1, -1)),
            VertexPosition(Vector3(-1, 1, -1)),
            VertexPosition(Vector3(-1, -1, 1)),
            VertexPosition(Vector3(1, -1, 1)),
            VertexPosition(Vector3(1, 1, 1)),
            VertexPosition(Vector3(-1, 1, 1))
        };
    }

    void GenerateSkyboxMesh(Mesh* mesh)
    {
        std::vector<DirectX::VertexPosition> verts;
        std::vector<uint32_t> indices;
        ComputeBox(verts, indices);

        mesh->index_count = indices.size();

        mesh->vertex_count = verts.size();
        mesh->vertex_stride = sizeof(DirectX::VertexPosition);
        mesh->vertex_offset = 0;

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = verts.size() * sizeof(DirectX::VertexPosition);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA subresourceData = { verts.data() };

        HRESULT hResult = engine::device->CreateBuffer(&vertexBufferDesc, &subresourceData, &mesh->vBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = mesh->index_count * sizeof(uint32_t);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices.data() };

        hResult = engine::device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &mesh->iBuffer);
        assert(SUCCEEDED(hResult));
    }

	ENGINE_API Renderer::Renderer() : m_MatrixBuffer(device, sizeof(MatrixBuffer))
	{
        camera = nullptr;
        context = nullptr;

        m_SkyboxShader = new Shader();
        const wchar_t* file = L"shaders/skybox.hlsl";
        m_SkyboxShader->Load(file, file, device);

        m_SkyboxMesh = new Mesh();
        GenerateSkyboxMesh(m_SkyboxMesh);
        device->CreateInputLayout(
            DirectX::VertexPosition::InputElements, 
            DirectX::VertexPosition::InputElementCount, 
            m_SkyboxShader->vsBlob->GetBufferPointer(), 
            m_SkyboxShader->vsBlob->GetBufferSize(), 
            &m_SkyboxInputLayout);
	}
}
