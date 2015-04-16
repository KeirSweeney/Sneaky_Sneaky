#pragma once

#include "LogicComponent.h"

class InteractablePoster : public Urho3D::LogicComponent
{
	OBJECT(InteractablePoster)

public:
	InteractablePoster(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);
};
