#pragma once
#include "common.h"
#include "globals.h"
#include "model.h"
#include "camera.h"
#include "constant_buffer.h"

namespace engine
{

	class ENGINE_API Renderer
	{
	public:
        static Renderer& Get();

        void SetContext(DeviceContext** context);
        void SetCamera(Camera* camera);
        void PreUpdate();
        void DrawSkybox(CubemapTexture* texture);
		void SubmitForRendering(Model* model, bool wireframe = false);

        Camera* camera;
        Shader* m_SkyboxShader;
	private:
        Renderer();

        DeviceContext** context;

        struct MatrixBuffer
        {
            Matrix model;
            Matrix view;
            Matrix projection;
            Matrix mvp;
            Color color;
            Vector3 camera;
            Vector3 cameraView;

            Vector2 pad;
        };

        Matrix m_ViewProjection;
        ConstantBuffer m_MatrixBuffer;

        Mesh* m_SkyboxMesh;
        ID3D11InputLayout* m_SkyboxInputLayout;
	};
}