#pragma once

#include "Urho3D/Scene/LogicComponent.h"

namespace Urho3D {
	class UIElement;
	class Text;
}

class CameraController: public Urho3D::LogicComponent
{
	OBJECT(CameraController)

public:
	CameraController(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);
    
public:
	void DelayedStart();
	void Update(float timeStep);

public:
	float GetYawAngle();

private:
	Urho3D::IntVector2 cameraRoom_;
	float cameraYaw_;
	float targetCameraYaw_;
	bool showPanel_;
	float panelYPosition_;
	Urho3D::UIElement *panel_;
	Urho3D::Text *label_;
};
