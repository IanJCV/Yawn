#pragma once
#include "common.h"
#include "shader.h"
#include "texture.h"

namespace engine
{
    class ENGINE_API AABB
    {
    public:
        DirectX::SimpleMath::Vector3 min;
        DirectX::SimpleMath::Vector3 max;
    };

    class ENGINE_API Mesh
    {
    public:
        ID3D11Buffer* vBuffer;
        ID3D11Buffer* iBuffer;

        UINT vertex_count;
        UINT vertex_stride;
        UINT vertex_offset;

        UINT index_count;

        AABB boundingBox;
        Texture* texture;
    };

    class ENGINE_API Model
    {
    public:
        std::vector<Mesh*> meshes;
        uint32_t meshCount;
        uint32_t indexCount;
        ID3D11InputLayout* layout = NULL;

        std::vector<ID3D11Buffer*> vBuffers;
        std::vector<UINT> strides;
        std::vector<UINT> offsets;
        std::vector<ID3D11Buffer*> iBuffers;

        Shader* shader;

        DirectX::SimpleMath::Matrix modelMatrix;
        DirectX::BoundingOrientedBox bounds;

        void SetTexture(Texture* tex);
        void Scale(float scale);
        void Translate(DirectX::SimpleMath::Vector3 movement);
        void Rotate(DirectX::SimpleMath::Vector3 eulers);
    };

    ENGINE_API engine::Model* LoadModel(const char* filename, engine::Shader* shader);
}