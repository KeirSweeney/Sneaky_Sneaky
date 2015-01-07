#pragma once

#include "LogicComponent.h"

class Pickup: public Urho3D::LogicComponent
{
    OBJECT(Pickup)

private:
    static const float ROTATION_SPEED;
    static const float HEIGHT_MIN;
    static const float HEIGHT_MAX;
	Urho3D::Node *trigger_;

public:
    Pickup(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);
	void SetTriggerNode(Urho3D::Node *trigger);
	Urho3D::Node *GetTriggerNode() const;

protected:
    float height_;

};
