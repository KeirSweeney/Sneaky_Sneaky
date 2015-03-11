#include "CameraController.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Input.h"
#include "Camera.h"
#include "Graphics.h"
#include "Octree.h"

using namespace Urho3D;

CameraController::CameraController(Context *context):
	LogicComponent(context), cameraRoom_(),
    cameraYaw_(0.0f), targetCameraYaw_(0.0f)
{
}

void CameraController::RegisterObject(Context *context)
{
    context->RegisterFactory<CameraController>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void CameraController::DelayedStart()
{
	Vector3 position = GetScene()->GetChild("Person", true)->GetPosition();
	cameraRoom_ = IntVector2((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
}

void CameraController::Update(float timeStep)
{
    Input *input = GetSubsystem<Input>();

    if (input->GetKeyPress('Q'))
        targetCameraYaw_ += 90.0f;
    if (input->GetKeyPress('E'))
        targetCameraYaw_ -= 90.0f;

	Vector3 position = GetScene()->GetChild("Person", true)->GetPosition();
	IntVector2 room((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));

	// This is glitchy, sometimes it travels the long way around.
	if (room.x_ < cameraRoom_.x_) { // Left
		targetCameraYaw_ = -90.0f;
	} else if (room.x_ > cameraRoom_.x_) { // Right
		targetCameraYaw_ = 90.0f;
	} else if (room.y_ < cameraRoom_.y_) { // Down
		targetCameraYaw_ = 180.0f;
	} else if (room.y_ > cameraRoom_.y_) { // Up
		targetCameraYaw_ = 0.0f;
	}

	cameraRoom_ = room;

    cameraYaw_ += (targetCameraYaw_ - cameraYaw_) * 5.0f * timeStep;
    node_->SetRotation(Quaternion(0.0f, cameraYaw_, 0.0f));

    // Snap the camera target to the center of the current room the player is in.
	position.x_ = room.x_ * 11.0f;
    position.y_ = 0.0f;
	position.z_ = room.y_ * 11.0f;

    // Lerp the camera towards the target position.
    Vector3 cameraPosition = node_->GetPosition();
    node_->SetPosition(cameraPosition + (position - cameraPosition) * 2.0f * timeStep);
}

float CameraController::GetYawAngle()
{
    return cameraYaw_;
}
