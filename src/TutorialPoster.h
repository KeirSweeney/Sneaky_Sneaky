#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class Material;
}

class TutorialPoster: public InteractableComponent
{
	OBJECT(TutorialPoster)

public:
	TutorialPoster(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

	void LoadFromXML(const Urho3D::XMLElement &xml);

public:
	void DelayedStart();
	void Update(float timeStep);

private:
	Urho3D::SharedPtr<Urho3D::Material> material_;
	Urho3D::SharedPtr<Urho3D::Material> controllerMaterial_;
};
