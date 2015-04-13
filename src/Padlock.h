#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
    class UIElement;
}

class Pickup;

class Padlock: public InteractableComponent
{
    OBJECT(Padlock)

private:
    static const float VIEW_DISTANCE;
    static const float VIEW_ANGLE;

public:
    Padlock(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

    void LoadFromXML(const Urho3D::XMLElement &xml);
    
public:
    void DelayedStart();
    void Update(float timeStep);

private:
    bool CanPlayerInteract();

private:
    int displayWidth_;
    int displayHeight_;
    Urho3D::String content_;
    Urho3D::UIElement *panel_;
};
