#include "Analytics.h"

#include "Network.h"
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

void Analytics::SendGameEvent(String event, std::initializer_list<EventKeyValue> data)
{
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

    Network *network = GetSubsystem<Network>();
    network->MakeHttpRequest(API_ENDPOINT, "POST", {}, (const char *)output.GetData());
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
