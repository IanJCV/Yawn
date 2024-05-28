#pragma once
#include "common.h"
#include "globals.h"
#include "shader.h"
#include "camera.h"
#include "constant_buffer.h"

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

        Matrix modelMatrix;
    };

	class ENGINE_API Renderer
	{
	public:
        static Renderer& Get();

        void SetContext(DeviceContext** context);
        void SetCamera(Camera* camera);
        void PreUpdate();
		void SubmitForRendering(Model* model);

        Camera* camera;
	private:
        Renderer();

        DeviceContext** context;

        struct MatrixBuffer
        {
            Matrix model;
            Matrix mvp;
            Color color;
            Vector3 camera;

            float pad;
        };

        Matrix m_ViewProjection;

        ConstantBuffer m_MatrixBuffer;
	};

    ENGINE_API engine::Model* LoadModel(const char* filename, engine::Shader* shader);
}