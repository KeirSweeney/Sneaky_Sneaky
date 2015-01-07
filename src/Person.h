#pragma once

#include "LogicComponent.h"

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

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
};
