#pragma once

#include "LogicComponent.h"

namespace Urho3D {
	class Material;
}

class ClickMarker: public Urho3D::LogicComponent
{
	OBJECT(ClickMarker)

public:
	ClickMarker(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void Start();
    void Update(float timeStep);

private:
	Urho3D::SharedPtr<Urho3D::Material> material_;
};
