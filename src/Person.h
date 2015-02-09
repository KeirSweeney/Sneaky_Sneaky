#pragma once

#include "LogicComponent.h"

namespace Urho3D {
	class Material;
}

class Person: public Urho3D::LogicComponent
{
    OBJECT(Person)

private:
    static const float MOVE_SPEED;

public:
    Person(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

public:
    void SetTarget(Urho3D::Vector3 target);
    Urho3D::Vector3 GetDirection() const;

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
	Urho3D::Material *frontMaterial_;
	Urho3D::Material *backMaterial_;
	Urho3D::Material *leftMaterial_;
	Urho3D::Material *rightMaterial_;
    Urho3D::Material *frontShadowMaterial_;
    Urho3D::Material *leftShadowMaterial_;
    Urho3D::Vector3 direction_;
};
