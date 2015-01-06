#pragma once

#include "LogicComponent.h"

class Pickup: public Urho3D::LogicComponent
{
    OBJECT(Pickup)

private:
    static constexpr float ROTATION_SPEED = 30.0f;
    static constexpr float HEIGHT_MIN = 0.2f;
    static constexpr float HEIGHT_MAX = 0.4f;

public:
    Pickup(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

protected:
    float height_;
};
