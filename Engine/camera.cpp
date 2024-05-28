#include "camera.h"
#include "globals.h"

using namespace DirectX::SimpleMath;

engine::Camera::Camera(Vector3 position, Quaternion rotation)
{
	this->Position = position;
	Matrix m = Matrix::CreateFromQuaternion(rotation);
	this->Forward = m.Forward();
	this->Right = m.Right();

	RecalculateProjection();
	this->View = Matrix::CreateLookAt(Position, Position + Forward, Vector3::UnitY);
}

void engine::Camera::SetFOV(float deg)
{
	this->FOV = deg;
}

void engine::Camera::RecalculateProjection()
{
	this->Projection = Matrix::CreatePerspectiveFieldOfView(deg2rad(FOV), float(g_WindowWidth) / float(g_WindowHeight), 0.01f, 1000.0f);
}

void engine::Camera::Update(float dt, Vector3 movement, Vector2 rotation)
{
	Pitch -= float(rotation.y) * Sensitivity;
	Yaw -= float(rotation.x) * Sensitivity;

    auto m = Matrix::CreateFromYawPitchRoll(Yaw, Pitch, 0.f);
    Forward = m.Forward();
    Right = m.Right();

    if (Pitch > d2r(88))
        Pitch = d2r(88);
    if (Pitch < -d2r(88))
        Pitch = -d2r(88);

    const float CAM_MOVE_AMOUNT = 2.f * dt;
	Position += (movement.z * Forward + movement.x * Right) * CAM_MOVE_AMOUNT;
	Position += (Vector3::UnitY * movement.y) * CAM_MOVE_AMOUNT;

	View = Matrix::CreateLookAt(Position, Position + Forward, Vector3::UnitY);
}

Matrix engine::Camera::Transform(Matrix model)
{
	return Matrix();
}
