#pragma once

#include "LogicComponent.h"

class InteractableComponent: public Urho3D::LogicComponent
{
    OBJECT(InteractableComponent)

public:
    InteractableComponent(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);
    
public:
    virtual void LoadFromXML(const Urho3D::XMLElement &xml);
};
