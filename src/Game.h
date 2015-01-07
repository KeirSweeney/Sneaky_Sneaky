#pragma once

#include "Application.h"

namespace Urho3D {
    class DebugHud;
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

public:
    void LoadLevel(bool next);

private:
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
    Urho3D::SharedPtr<Urho3D::DebugHud> debugHud_;
    Urho3D::SharedPtr<Urho3D::Scene> scene_;
    int currentLevel_;

    bool debugGeometry_;
    bool debugPhysics_;
    bool debugNavigation_;
    bool debugDepthTest_;
};
