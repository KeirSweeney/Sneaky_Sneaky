#pragma once

#include "InteractableComponent.h"

class InteractablePoster : public InteractableComponent
{
	OBJECT(InteractablePoster)

public:
	InteractablePoster(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);
};
