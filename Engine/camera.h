#pragma once
#include "common.h"

namespace engine
{
	namespace
	{
		using namespace DirectX::SimpleMath;
	}

	class ENGINE_API Camera
	{
	public:
		Camera(Vector3 position, Quaternion rotation);

		void SetFOV(float deg);

		void RecalculateProjection();

		void Update(float dt, Vector3 movement, Vector2 rotation);

		Matrix Transform(Matrix model);

		Vector3 Position;
		Vector3 Forward;
		Vector3 Right;
		Matrix Projection;
		Matrix View;
		float Pitch = 0.f;
		float Yaw = 0.f;
		float Sensitivity = 0.002f;
		float FOV = 90.f;
	private:
	};
}