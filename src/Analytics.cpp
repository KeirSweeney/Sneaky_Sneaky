#include "Analytics.h"

#include "Network.h"
#include "Log.h"
#include "HttpRequest.h"
#include "VectorBuffer.h"
#include "JSONFile.h"
#include "StringUtils.h"

using namespace Urho3D;

const String Analytics::API_ENDPOINT = "http://traitor.limetech.org/analytics.php";

Analytics::Analytics(Context *context):
    Object(context)
{
}

Analytics::~Analytics()
{
    FlushEvents();
}

void Analytics::FlushEvents()
{
    for (Vector<HttpRequest *>::Iterator i = requests_.Begin(); i != requests_.End(); ++i) {
        (*i)->ReleaseRef();
    }
}

void Analytics::SendLaunchEvent()
{
    SendGameEvent("launch", {
        {"compile-date", __DATE__},
        {"compile-time", __TIME__},
        {"platform", CMAKE_SYSTEM_NAME},
        {"generator", CMAKE_GENERATOR},
        {"compiler", CMAKE_CXX_COMPILER_ID},
    });
}

void Analytics::SendLevelCompletedEvent(int level, float levelTime, int guardCount, int pickupCount, int score)
{
    SendGameEvent("level-complete", {
        {"level", String(level)},
        {"level-time", String(levelTime)},
        {"guard-count", String(guardCount)},
        {"pickup-count", String(pickupCount)},
        {"score", String(score)},
    });
}

void Analytics::SendLevelFailedEvent(int level, float levelTime, int guardCount, int pickupCount, Vector3 playerPosition)
{
    SendGameEvent("level-failed", {
        {"level", String(level)},
        {"level-time", String(levelTime)},
        {"guard-count", String(guardCount)},
        {"pickup-count", String(pickupCount)},
        {"player-position", playerPosition.ToString()},
    });
}

void Analytics::SendCrashEvent(String id)
{
    SendGameEvent("crash", {
        {"id", id.CString()},
    });
}

void Analytics::SendGameEvent(String event, std::initializer_list<EventKeyValue> data)
{
    // First, clean up any finished requests. If we don't store them, it blocks in the dtor.
    for (Vector<HttpRequest *>::Iterator i = requests_.Begin(); i != requests_.End();) {
        HttpRequestState state = (*i)->GetState();

        if (state != HTTP_ERROR && state != HTTP_CLOSED) {
            i++;
            continue;
        }

        if (state == HTTP_ERROR) {
            LOGERRORF("HTTP Request Error: %s", (*i)->GetError().CString());
        }

        i = requests_.Erase(i);
    }

    SharedPtr<JSONFile> json(new JSONFile(context_));

    JSONValue root = json->CreateRoot();
    root.SetString("event", event);

    JSONValue eventData = root.CreateChild("data");
    for (EventKeyValue item : data) {
        eventData.SetString(item.key, item.value);
    }

    VectorBuffer output;
    json->Save(output);
    output.WriteByte('\0');

    HttpRequest *request = new HttpRequest(API_ENDPOINT, "POST", {}, (const char *)output.GetData());
    request->AddRef();
    requests_.Push(request);
}
