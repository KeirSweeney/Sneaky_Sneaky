#pragma once

#include "Object.h"

#include <initializer_list>

class Analytics: public Urho3D::Object
{
    OBJECT(Analytics)

private:
    static const Urho3D::String API_ENDPOINT;

public:
    Analytics(Urho3D::Context *context);

private:
    struct EventKeyValue {
        Urho3D::String key;
        Urho3D::String value;
    };

    void SendGameEvent(Urho3D::String event, std::initializer_list<EventKeyValue> data);

public:
    void SendLaunchEvent();
    void SendLevelCompletedEvent(int level, float levelTime, int guardCount, int pickupCount, int score);
    void SendLevelFailedEvent(int level, Urho3D::Vector3 playerPosition);
};
