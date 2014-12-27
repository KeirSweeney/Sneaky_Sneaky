#pragma once

#include "LogicComponent.h"

class Pickup: public Urho3D::LogicComponent
{
    OBJECT(Pickup)

public:
    Pickup(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

protected:
    float height_;
};
