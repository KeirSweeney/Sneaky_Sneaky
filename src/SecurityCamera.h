#pragma once

#include "InteractableComponent.h"

class SecurityCamera: public InteractableComponent
{
    OBJECT(SecurityCamera)

public:
	SecurityCamera(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

public:
    void DelayedStart();
    void Update(float timeStep);
	bool SearchForPlayer(Urho3D::Node* personNode);
	void AlertGuards();

private:
    bool sweepingBack_;
    Urho3D::Quaternion startRotation_;
    float pitch_;
    float yaw_;
	Urho3D::PODVector<Urho3D::Vector3> path_;
};
