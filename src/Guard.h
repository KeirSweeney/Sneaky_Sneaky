#pragma once

#include "LogicComponent.h"

namespace Urho3D {
    class RigidBody;
    class Material;
}

class Guard:public Urho3D::LogicComponent
{
    OBJECT(Guard)

private:
    static const float MOVE_SPEED;
	static const float VIEW_DISTANCE;
	static const float VIEW_ANGLE;
    static const float DETECT_MOVE_SPEED;

public:
    Guard(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

public:
    void DelayedStart();
    void Update(float timeStep);

private:
    bool DetectPlayer(Urho3D::Node *player);
    void FollowWaypoints(float timeStep);
    void FollowPlayer(float timeStep, Urho3D::Node *player);

public:
    void SetWaypoints(Urho3D::PODVector<Urho3D::Vector3> &waypoints);
    bool HasSeenPlayer();

private:
    Urho3D::RigidBody *rigidBody_;
    Urho3D::PODVector<Urho3D::Vector3> path_;
    Urho3D::PODVector<Urho3D::Vector3> waypoints_;
    bool hasSeenPlayer_;
    bool wasFollowingPlayer_;
    Urho3D::Material *frontMaterial_;
    Urho3D::Material *backMaterial_;
    Urho3D::Material *leftMaterial_;
    Urho3D::Material *rightMaterial_;
};
