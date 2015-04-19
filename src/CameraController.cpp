#include "CameraController.h"

#include "Game.h"

#include "Camera.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Graphics.h"
#include "Input.h"
#include "Node.h"
#include "Octree.h"
#include "Scene.h"
#include "Sprite.h"
#include "StringUtils.h"
#include "Text.h"
#include "UI.h"
#include "UIElement.h"

using namespace Urho3D;

CameraController::CameraController(Context *context):
	LogicComponent(context), cameraRoom_(),
	cameraYaw_(0.0f), targetCameraYaw_(0.0f),
	panelYPosition_(50.0f)
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

	const Variant &roomName = GetScene()->GetChild(ToString("%dx%d", cameraRoom_.x_ + 1, cameraRoom_.y_ + 1))->GetVar("label");

	UI *ui = GetSubsystem<UI>();

	showPanel_ = !roomName.IsEmpty();

	panel_ = ui->GetRoot()->CreateChild<UIElement>();
	panel_->SetFixedSize(panel_->GetParent()->GetWidth(), 50);
	panel_->SetVerticalAlignment(VA_BOTTOM);
	panel_->SetPosition(0, (int)panelYPosition_);

	Sprite *background = panel_->CreateChild<Sprite>();
	background->SetFixedSize(panel_->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.6f);

	label_ = panel_->CreateChild<Text>();
	label_->SetFixedSize(panel_->GetSize() - IntVector2(20, 20));
	label_->SetFont("Fonts/Anonymous Pro.ttf", 24);
	label_->SetColor(Color::WHITE);
	label_->SetTextAlignment(HA_CENTER);
	label_->SetAlignment(HA_CENTER, VA_CENTER);

	if (!roomName.IsEmpty()) {
		label_->SetText(roomName.GetString());
	}
}

void CameraController::Update(float timeStep)
{
	Input *input = GetSubsystem<Input>();

	if (input->GetKeyPress('Q')) {
		targetCameraYaw_ -= 90.0f;
	}

	if (input->GetKeyPress('E')) {
		targetCameraYaw_ += 90.0f;
	}

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

	if (room != cameraRoom_) {
		const Variant &roomName = GetScene()->GetChild(ToString("%dx%d", room.x_ + 1, room.y_ + 1))->GetVar("label");

		showPanel_ = !roomName.IsEmpty();

		if (!roomName.IsEmpty()) {
			label_->SetText(roomName.GetString());
		}
	}

	if (showPanel_ && panelYPosition_ > 0.0f) {
		panelYPosition_ -= timeStep * 50.0f;
	} else if (!showPanel_ && panelYPosition_ < 50.0f) {
		panelYPosition_ += timeStep * 50.0f;
	}

	panel_->SetPosition(0, (int)panelYPosition_);

	cameraRoom_ = room;

	cameraYaw_ += (targetCameraYaw_ - cameraYaw_) * 5.0f * timeStep;
	node_->SetRotation(Quaternion(0.0f, cameraYaw_, 0.0f));

	// Snap the camera target to the center of the current room the player is in.
	position.x_ = room.x_ * 11.0f;
	position.y_ = 0.0f;
	position.z_ = room.y_ * 11.0f;

	if (GetSubsystem<Game>()->IsDeveloper() && input->GetKeyDown(KEY_W)) {
		position += node_->GetDirection() * 5.5f;
		position.y_ += 2.0f;
	}

	// Lerp the camera towards the target position.
	Vector3 cameraPosition = node_->GetPosition();
	node_->SetPosition(cameraPosition + (position - cameraPosition) * 2.0f * timeStep);
}

float CameraController::GetYawAngle()
{
	return cameraYaw_;
}
