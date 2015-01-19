#pragma once

#include "LogicComponent.h"

class Thrower: public Urho3D::LogicComponent
{
    OBJECT(Thrower)


public:
    Thrower(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);



};
