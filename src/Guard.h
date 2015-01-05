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
    bool DetectPlayer();

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
    Urho3D::PODVector<Urho3D::Vector3> waypoints_;


};
