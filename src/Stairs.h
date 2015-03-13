#pragma once

#include "InteractableComponent.h"

class Stairs: public InteractableComponent
{
    OBJECT(Stairs)

public:
    Stairs(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);
    
public:
    void DelayedStart();
    void Update(float timeStep);
};
