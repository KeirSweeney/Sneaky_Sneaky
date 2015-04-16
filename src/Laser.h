#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class RigidBody;
}

class Laser:public InteractableComponent
{
	OBJECT(Laser)

public:
	Laser(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);
	void LoadFromXML(const Urho3D::XMLElement &xml);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
	Urho3D::RigidBody *rigidBody_;
	bool lightPulse_;
	float lightTime_;
	float laserTime_;
	float laserInterval_;
};
