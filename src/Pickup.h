#pragma once

#include "InteractableComponent.h"

class Pickup: public InteractableComponent
{
    OBJECT(Pickup)

private:
    static const float ROTATION_SPEED;
    static const float HEIGHT_MIN;
    static const float HEIGHT_MAX;

public:
    Pickup(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);
    
public:
    void DelayedStart();
    void Update(float timeStep);
    void LoadFromXML(const Urho3D::XMLElement &xml);

public:
    Urho3D::String GetPickupType();

protected:
    float height_;
    Urho3D::String type_;
};
