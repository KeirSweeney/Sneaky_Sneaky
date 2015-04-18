#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class Material;
	class Texture2D;
}

class AnimatedPoster: public InteractableComponent
{
	OBJECT(AnimatedPoster)

public:
	AnimatedPoster(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

	void LoadFromXML(const Urho3D::XMLElement &xml);

public:
	void DelayedStart();
	void Update(float timeStep);

private:
	float speed_;
	int count_;
	Urho3D::String path_;

	float timer_;
	int frame_;
	Urho3D::SharedPtr<Urho3D::Material> material_;
};
