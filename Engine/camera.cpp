#include "camera.h"
#include "globals.h"

using namespace DirectX::SimpleMath;

engine::Camera::Camera(Vector3 position, Quaternion rotation)
{
	this->Position = position;
	Matrix m = Matrix::CreateFromQuaternion(rotation);
	this->Forward = m.Forward();
	this->Right = m.Right();

	this->Projection = Matrix::CreatePerspectiveFieldOfView(d2r(70.0f), float(g_WindowWidth) / float(g_WindowHeight), 0.01f, 100.0f);
	this->View = Matrix::CreateLookAt(Position, Position + Forward, Vector3::UnitY);
}

void engine::Camera::SetFOV(float deg)
{
	this->Projection = Matrix::CreatePerspectiveFieldOfView(deg2rad(deg), float(g_WindowWidth) / float(g_WindowHeight), 0.01f, 100.0f);
}

void engine::Camera::Update(Vector3 movement, Vector2 rotation)
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

    const float CAM_MOVE_SPEED = 2.f; // in metres per second
    const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
    Position += movement.x * Forward + movement.y * Right;
	Position += Vector3::UnitY * movement.y;
}

Matrix engine::Camera::Transform(Matrix model)
{
	return Matrix();
}
