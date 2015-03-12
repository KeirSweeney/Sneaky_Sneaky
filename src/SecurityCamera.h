#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
    class RigidBody;
    class Material;
}

class SecurityCamera: public InteractableComponent
{
	OBJECT(SecurityCamera)

private:
	static const float VIEW_DISTANCE;
	static const float VIEW_ANGLE;
	float CAMERA_ROT_SPEED = 15.0f;
	float CAMERA_START_ROT;

public:
	SecurityCamera(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

public:
    void DelayedStart();
    void Update(float timeStep);

private:
    bool DetectPlayer(Urho3D::Node *player);
	void Seeking(float timeStep);
	//Follow player in terms of angle
	void CameraRotateOnPlayer(float timeStep, Urho3D::Node *player);

public:
    bool HasSeenPlayer();

private:
    Urho3D::RigidBody *rigidBody_;
    //Urho3D::PODVector<Urho3D::Vector3> path_;
    //Urho3D::PODVector<Urho3D::Vector3> waypoints_;
    bool hasSeenPlayer_;
    bool rotatingToPlayer_;
    Urho3D::Material *frontMaterial_;
    Urho3D::Material *backMaterial_;
    Urho3D::Material *leftMaterial_;
    Urho3D::Material *rightMaterial_;
};
