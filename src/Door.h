#pragma once

#include "LogicComponent.h"

class Door: public Urho3D::LogicComponent
{
    OBJECT(Door)

private:
	static const float DOOR_OFFSET;

public:
    Door(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

public:
    void SetTriggerNode(Urho3D::Node *trigger);
    Urho3D::Node *GetTriggerNode() const;

private:
    Urho3D::Node *trigger_;
};
