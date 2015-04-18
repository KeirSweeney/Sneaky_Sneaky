#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class StaticModel;
	class Material;
}

class MrWright : public InteractableComponent
{
	OBJECT(MrWright)

public:
	MrWright(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
	float swapTimer_;
	Urho3D::Vector<Urho3D::PODVector<Urho3D::Material *>> sequences_;
};
