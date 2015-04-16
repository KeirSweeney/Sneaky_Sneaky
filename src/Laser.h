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
    void Update(float timeStep);
	void Start();
    void DelayedStart();
    void LoadFromXML(const Urho3D::XMLElement &xml);

public:
    void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
    Urho3D::RigidBody *rigidBody_;
    Urho3D::PODVector<Urho3D::Vector3> path_;
    bool lightPulse_;
    float lightTime_;
    float laserTime_;
    float laserInterval_;
	float laserDelay_;
};
