#pragma once

#include "LogicComponent.h"

class Stairs: public Urho3D::LogicComponent
{
    OBJECT(Stairs)

public:
    Stairs(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);
};
