#pragma once

#include "LogicComponent.h"

namespace Urho3D {
	class RigidBody;
}

class Laser:public Urho3D::LogicComponent
{
	OBJECT(Laser)

public:
	Laser(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
    void Update(float timeStep);
    void DelayedStart();

public:
    void HandleNodeCollisionStart(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);


private:
    Urho3D::RigidBody *rigidBody_;
    Urho3D::PODVector<Urho3D::Vector3> path_;
    bool lightPulse_;
    float lightTime_;
};
