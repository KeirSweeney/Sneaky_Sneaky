#pragma once

#include "LogicComponent.h"

namespace Urho3D {
    class UIElement;
}

class Terminal: public Urho3D::LogicComponent
{
    OBJECT(Terminal)

private:
    static const float VIEW_DISTANCE;
    static const float VIEW_ANGLE;

public:
    Terminal(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

private:
    bool CanPlayerInteract();

private:
    Urho3D::UIElement *panel_;
};
