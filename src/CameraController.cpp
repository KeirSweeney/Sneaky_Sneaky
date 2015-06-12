#include "Urho3D/Urho3D.h"

#include "CameraController.h"

#include "Game.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Core/StringUtils.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/UI/UIElement.h"

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
	float pixelRatio = GetSubsystem<Graphics>()->GetPixelRatio();

	showPanel_ = !roomName.IsEmpty();

	panel_ = ui->GetRoot()->CreateChild<UIElement>();
	panel_->SetFixedSize(panel_->GetParent()->GetWidth(), 50 * pixelRatio);
	panel_->SetVerticalAlignment(VA_BOTTOM);
	panel_->SetPosition(0, (int)panelYPosition_);

	Sprite *background = panel_->CreateChild<Sprite>();
	background->SetFixedSize(panel_->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.6f);

	label_ = panel_->CreateChild<Text>();
	label_->SetFixedSize(panel_->GetSize() - (IntVector2(20, 20) * pixelRatio));
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
	
	bool rotateLeft = false;
	bool rotateRight = false;
	
	if (input->GetNumJoysticks() > 0) {
		JoystickState *state = input->GetJoystickByIndex(0);
		
		rotateLeft = state->GetButtonPress(9);
		rotateRight = state->GetButtonPress(10);
	}
	
	rotateLeft = rotateLeft || input->GetKeyPress('Q');
	rotateRight = rotateRight || input->GetKeyPress('E');

	if (rotateLeft) {
		targetCameraYaw_ -= 90.0f;
	}

	if (rotateRight) {
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

	float pixelRatio = GetSubsystem<Graphics>()->GetPixelRatio();
	panel_->SetPosition(0, (int)(panelYPosition_ * pixelRatio));

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
