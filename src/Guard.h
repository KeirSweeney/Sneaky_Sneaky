#pragma once

#include "LogicComponent.h"

class Guard:public Urho3D::LogicComponent
{
    OBJECT(Guard)

private:
    static constexpr float MOVE_SPEED = 1.0f;
    static constexpr float VIEW_DISTANCE = 5.0f;
    static constexpr float VIEW_ANGLE = 90.0f;

public:
    Guard(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

public:
    void Update(float timeStep);

private:
    bool DetectPlayer(Urho3D::Node *player);
    void FollowWaypoints(float timeStep);
    void FollowPlayer(float timeStep, Urho3D::Node *player);

public:
    void SetWaypoints(Urho3D::PODVector<Urho3D::Vector3> &waypoints);

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
    Urho3D::PODVector<Urho3D::Vector3> waypoints_;
};
