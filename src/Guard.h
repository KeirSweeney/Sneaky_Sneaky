#pragma once

#include "LogicComponent.h"

class Guard:public Urho3D::LogicComponent
{
    OBJECT(Guard)


    public:
        Guard(Urho3D::Context *context);
        static void RegisterObject(Urho3D::Context *context);

    public:
        void Update(float timeStep);

public:
    void SetWaypoints(Urho3D::PODVector<Urho3D::Vector3>  &waypoint);
    bool DetectPlayer(Urho3D::Node *myChar);
    void FollowWaypoints(float timeStep);
    void FollowPlayer(float timeStep, Urho3D::Node *player);

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
    Urho3D::PODVector<Urho3D::Vector3> waypoints_;


};
