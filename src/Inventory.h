#pragma once

#include "Urho3D/Scene/LogicComponent.h"

namespace Urho3D {
	class UIElement;
}

class Pickup;

class Inventory: public Urho3D::LogicComponent
{
	OBJECT(Inventory)

private:
	static const int PADDING;

public:
	Inventory(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);

public:
	void AddItem(Pickup *item);
	int GetItemCount();
	Urho3D::SharedPtr<Pickup> GetItemOfType(Urho3D::String type, bool remove);

private:
	bool dirty_;
	Urho3D::UIElement *panel_;
	Urho3D::Vector<Urho3D::SharedPtr<Pickup>> items_;
};
