#pragma once

#include "Object.h"

#include <initializer_list>

namespace Urho3D {
    class HttpRequest;
}

class Analytics: public Urho3D::Object
{
    OBJECT(Analytics)

private:
    static const Urho3D::String API_ENDPOINT;

public:
    Analytics(Urho3D::Context *context);
    ~Analytics();

public:
    void FlushEvents();

    void SendLaunchEvent();
    void SendLevelCompletedEvent(int level, float levelTime, int guardCount, int pickupCount, int score);
    void SendLevelFailedEvent(int level, float levelTime, int guardCount, int pickupCount, Urho3D::Vector3 playerPosition);
    void SendCrashEvent(Urho3D::String id);

private:
    struct EventKeyValue {
        Urho3D::String key;
        Urho3D::String value;
    };

    void SendGameEvent(Urho3D::String event, std::initializer_list<EventKeyValue> data);

private:
    // Not using SharedPtr due to forward decl.
    // Can't bring in full decl as it conflicts with Breakpad.
    Urho3D::Vector<Urho3D::HttpRequest *> requests_;
};
