#pragma once

#include "LogicComponent.h"

class SelfDestroy: public Urho3D::LogicComponent
{
    OBJECT(SelfDestroy)

public:
    SelfDestroy(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

public:
    void SetLifeTime(float lifeTime);

private:
    float lifeTime_;
    float decayTime_;
};
