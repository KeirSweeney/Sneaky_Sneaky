#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class StaticModel;
	class Material;
}

class Wife : public InteractableComponent
{
	OBJECT(Wife)

public:
	Wife(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
	Urho3D::PODVector<int> sequence_;
	Urho3D::PODVector<Urho3D::Node *> terminals_;
	float time_;
};
