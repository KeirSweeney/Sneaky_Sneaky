#pragma once

#include "Urho3D/Scene/LogicComponent.h"

class Thrower: public Urho3D::LogicComponent
{
	OBJECT(Thrower)

public:
	Thrower(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void Update(float timeStep);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

public:
	void DistractGuard(Urho3D::Node *itemNode);
};
