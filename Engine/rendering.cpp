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
        camera->Projection.Transpose(proj);
        camera->View.Transpose(v);

        buf->model = Matrix::Identity;
        buf->view = v;
        buf->projection = proj;
        buf->mvp = Matrix::Identity * camera->View * camera->Projection;
        buf->color = Color(1.f, 1.f, 1.f, 1.f);
        buf->camera = camera->Position;
        buf->pad = 1.f;

        m_MatrixBuffer.EndUpdate(ctx);

        ctx->IASetInputLayout(m_SkyboxInputLayout);

        m_MatrixBuffer.Use(ctx);

        ctx->IASetVertexBuffers(0, 1, &m_SkyboxMesh->vBuffer, &m_SkyboxMesh->vertex_stride, &m_SkyboxMesh->vertex_offset);
        ctx->IASetIndexBuffer(m_SkyboxMesh->iBuffer, DXGI_FORMAT_R32_UINT, 0);
        ctx->DrawIndexed(m_SkyboxMesh->index_count, 0, 0);
    }

	ENGINE_API void Renderer::SubmitForRendering(Model* model)
	{
		auto ctx = (*context);
		
		ctx->VSSetShader(model->shader->vertexShader, NULL, 0);
		ctx->PSSetShader(model->shader->pixelShader, NULL, 0);

        if (model->texture)
        {
            ctx->PSSetShaderResources(0, 1, &model->texture->textureView);
        }

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
        buf->pad = 1.f;

        m_MatrixBuffer.EndUpdate(ctx);

		ctx->IASetInputLayout(model->layout);

        m_MatrixBuffer.Use(ctx);

        for (size_t i = 0; i < model->meshCount; i++)
		{
            Mesh* mesh = model->meshes[i];

			ctx->IASetVertexBuffers(0, 1, &mesh->vBuffer, &mesh->vertex_stride, &mesh->vertex_offset);

			ctx->IASetIndexBuffer(mesh->iBuffer, DXGI_FORMAT_R32_UINT, 0);

			ctx->DrawIndexed(mesh->index_count, 0, 0);
        }
	}

    void ComputeBox(std::vector<DirectX::VertexPosition>& vertices, std::vector<uint32_t>& indices)
    {
        vertices.clear();
        indices.clear();

        using namespace DirectX;

        DirectX::XMVECTORF32 g_XMIdentityR1 = { { { 0.0f, 1.0f, 0.0f, 0.0f } } };
        DirectX::XMVECTORF32 g_XMIdentityR2 = { { { 0.0f, 0.0f, 1.0f, 0.0f } } };


        // A box has six faces, each one pointing in a different direction.
        constexpr int FaceCount = 6;

        static const DirectX::XMVECTORF32 faceNormals[FaceCount] =
        {
            { { {  0,  0,  1, 0 } } },
            { { {  0,  0, -1, 0 } } },
            { { {  1,  0,  0, 0 } } },
            { { { -1,  0,  0, 0 } } },
            { { {  0,  1,  0, 0 } } },
            { { {  0, -1,  0, 0 } } },
        };

        static const DirectX::XMVECTORF32 textureCoordinates[4] =
        {
            { { { 1, 0, 0, 0 } } },
            { { { 1, 1, 0, 0 } } },
            { { { 0, 1, 0, 0 } } },
            { { { 0, 0, 0, 0 } } },
        };

        // Create each face in turn.
        for (int i = 0; i < FaceCount; i++)
        {
            const DirectX::XMVECTOR normal = faceNormals[i];

            // Get two vectors perpendicular both to the face normal and to each other.
            const DirectX::XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

            const DirectX::XMVECTOR side1 = DirectX::XMVector3Cross(normal, basis);
            const DirectX::XMVECTOR side2 = DirectX::XMVector3Cross(normal, side1);

            // Six indices (two triangles) per face.
            const size_t vbase = vertices.size();
            indices.push_back(vbase + 0);
            indices.push_back(vbase + 1);
            indices.push_back(vbase + 2);

            indices.push_back(vbase + 0);
            indices.push_back(vbase + 2);
            indices.push_back(vbase + 3);

            // Four vertices per face.
            // (normal - side1 - side2) * tsize // normal // t0
            vertices.push_back(DirectX::VertexPosition(XMVectorSubtract(XMVectorSubtract(normal, side1), side2)));

            // (normal - side1 + side2) * tsize // normal // t1
            vertices.push_back(DirectX::VertexPosition(XMVectorAdd(XMVectorSubtract(normal, side1), side2)));

            // (normal + side1 + side2) * tsize // normal // t2
            vertices.push_back(DirectX::VertexPosition(XMVectorAdd(normal, XMVectorAdd(side1, side2))));

            // (normal + side1 - side2) * tsize // normal // t3
            vertices.push_back(DirectX::VertexPosition(XMVectorSubtract(XMVectorAdd(normal, side1), side2)));
        }
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

    engine::Model* LoadModel(const char* filename, engine::Shader* shader)
    {
        if (!std::filesystem::exists(filename))
        {
            return nullptr;
        }

        engine::Model* out;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filename,
            aiProcess_Triangulate |
            aiProcess_GenBoundingBoxes);

        if (scene == nullptr)
        {
            return nullptr;
        }

        out = new engine::Model();

        aiMesh* rMesh = nullptr;
        engine::Mesh* tMesh = nullptr;

        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            rMesh = scene->mMeshes[i];
            tMesh = new engine::Mesh();

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

            engine::AABB aabb;
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

            D3D11_SUBRESOURCE_DATA subresourceData = { verts.data() };

            HRESULT hResult = engine::device->CreateBuffer(&vertexBufferDesc, &subresourceData, &tMesh->vBuffer);
            assert(SUCCEEDED(hResult));

            D3D11_BUFFER_DESC indexBufferDesc = {};
            indexBufferDesc.ByteWidth = tMesh->index_count * sizeof(uint32_t);
            indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA indexSubresourceData = { indices };

            hResult = engine::device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &tMesh->iBuffer);
            assert(SUCCEEDED(hResult));

            out->meshes.push_back(tMesh);
            out->meshCount++;
            out->indexCount += tMesh->index_count;

            out->vBuffers.push_back(tMesh->vBuffer);
            out->iBuffers.push_back(tMesh->iBuffer);
            out->strides.push_back(tMesh->vertex_stride);
            out->offsets.push_back(tMesh->vertex_offset);
        }

        out->modelMatrix = Matrix::Identity;
        out->shader = shader;
        HRESULT res = device->CreateInputLayout(
            Vertex::InputElements, 
            Vertex::InputElementCount, 
            shader->vsBlob->GetBufferPointer(), 
            shader->vsBlob->GetBufferSize(), 
            &out->layout);

        assert(SUCCEEDED(res));

        if (FAILED(res))
        {
            delete out;
            delete scene;
            return nullptr;
        }

        return out;
    }
}
