#pragma once

#include "LogicComponent.h"

class CameraController: public Urho3D::LogicComponent
{
    OBJECT(CameraController)

public:
    CameraController(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

public:
    float GetYawAngle();

private:
    float cameraYaw_;
    float targetCameraYaw_;
};
