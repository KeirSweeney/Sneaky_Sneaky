#pragma once

#include "LogicComponent.h"
#include "Text.h"

namespace Urho3D {
    class UIElement;
}

class Inventory: public Urho3D::LogicComponent
{
    OBJECT(Inventory)

public:
    Inventory(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);
	void AddToInventory(bool itemAdded);
	

private:
    Urho3D::UIElement *panel_;
};
