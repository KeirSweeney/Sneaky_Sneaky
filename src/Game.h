#pragma once

#include "Application.h"

namespace Urho3D {
    class Scene;
    class Camera;
    class Node;
}

class Game: public Urho3D::Application
{
    OBJECT(Game)

public:
    Game(Urho3D::Context *context);

    void Setup();
    void Start();
    void Stop();

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    Urho3D::Node *camera_;
    float cameraYaw_;
    float targetCameraYaw_;
    Urho3D::Node *person_;
};
