#pragma once

#include "LogicComponent.h"

class Person: public Urho3D::LogicComponent
{
    OBJECT(Person)

public:
    Person(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

private:
    Urho3D::Vector3 target_;
    Urho3D::PODVector<Urho3D::Vector3> path_;
};
