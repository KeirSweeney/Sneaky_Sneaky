#pragma once

#include "Application.h"

namespace Urho3D {
    class Scene;
    class Camera;
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
    Urho3D::Camera *camera_;
    float yaw_;
    float pitch_;
};
