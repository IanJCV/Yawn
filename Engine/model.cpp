#include "model.h"
#include <filesystem>

namespace engine
{
    using namespace DirectX::SimpleMath;

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

                verts.push_back(Vertex(Vector3(pos.x, pos.y, pos.z), Vector3(norm.x, norm.y, norm.z), Color(color.r, color.g, color.b, color.a), Vector2(texcoord.x, 1.f - texcoord.y)));
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

            if (scene->HasMaterials())
            {
                aiMaterial* mat = scene->mMaterials[rMesh->mMaterialIndex];

                std::filesystem::path directory = std::filesystem::path(filename).parent_path();
                std::filesystem::path root = std::filesystem::current_path();

                if (mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0 || mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
                {
                    aiString path;
                    if ((mat->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn_SUCCESS
                        || mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
                        && std::string(filename).size() > 1)
                    {
                        std::string p(path.C_Str());
                        if (std::filesystem::path(path.C_Str()).is_relative())
                        {
                            p = ((root / directory) / p.c_str()).string();
                        }

                        if (std::filesystem::is_regular_file(p))
                        {
                            Texture* tex = Texture::GetTexture(p.c_str());

                            tMesh->texture = tex;
                        }
                    }
                }
            }

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

        Vector3 pos;
        out->modelMatrix.Translation(pos);

        Vector3 extents;

        for (size_t i = 0; i < out->meshCount; i++)
        {
            AABB a = out->meshes[i]->boundingBox;

            extents.x = max(extents.x, a.max.x / 2.f);
            extents.y = max(extents.y, a.max.y / 2.f);
            extents.z = max(extents.z, a.max.z / 2.f);
        }

        out->bounds = DirectX::BoundingOrientedBox(pos, extents, Quaternion::Identity);

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

    void Model::SetTexture(Texture* tex)
    {
        for (size_t i = 0; i < meshCount; i++)
        {
            meshes[i]->texture = tex;
        }
    }


    void Model::Rotate(Vector3 eulers)
    {
        modelMatrix *= Matrix::CreateFromYawPitchRoll(d2r(eulers));
    }

    void Model::Translate(Vector3 movement)
    {
        modelMatrix *= Matrix::CreateTranslation(movement);
    }

    void Model::Scale(float scale)
    {
        modelMatrix *= Matrix::CreateScale(scale);
    }
}
