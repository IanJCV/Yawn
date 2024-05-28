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

	ENGINE_API void Renderer::SubmitForRendering(Model* model)
	{
		auto ctx = (*context);
		
		ctx->VSSetShader(model->shader->vertexShader, NULL, 0);
		ctx->PSSetShader(model->shader->pixelShader, NULL, 0);

        void* c = nullptr;
        m_MatrixBuffer.BeginUpdate(ctx, &c);
        MatrixBuffer* buf = (MatrixBuffer*)c;
        buf->model = model->modelMatrix;
        buf->mvp = model->modelMatrix * camera->View * camera->Projection;
        buf->color = Color(1.f, 1.f, 1.f, 1.f);
        buf->camera = camera->Position;
        buf->pad = 1.f;
        m_MatrixBuffer.EndUpdate(ctx);

		ctx->IASetInputLayout(model->layout);

        m_MatrixBuffer.Use(ctx);

		ctx->IASetVertexBuffers(0, model->meshCount, model->vBuffers.data(), model->strides.data(), model->offsets.data());

        for (size_t i = 0; i < model->meshCount; i++)
        {
            ctx->IASetIndexBuffer(model->iBuffers[i], DXGI_FORMAT_R32_UINT, 0);

            ctx->DrawIndexed(model->meshes[i]->index_count, 0, 0);
        }
	}

	ENGINE_API Renderer::Renderer() : m_MatrixBuffer(device, sizeof(MatrixBuffer))
	{
        camera = nullptr;
        context = nullptr;
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
