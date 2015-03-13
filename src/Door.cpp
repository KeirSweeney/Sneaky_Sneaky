#include "Door.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Person.h"
#include "PhysicsEvents.h"

using namespace Urho3D;

const float Door::DOOR_OFFSET = -1.275f;

Door::Door(Context *context):
    LogicComponent(context),
    trigger_(nullptr)
{
}

void Door::RegisterObject(Context* context)
{
    context->RegisterFactory<Door>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Door::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
    Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();
    Node *person = GetScene()->GetChild("Person", true);
    if (other != person) {
        return;
    }

    node_->SetPosition(Vector3(DOOR_OFFSET, 0.0f, 0.0f));

    Vector3 position = GetScene()->GetChild("Person", true)->GetPosition();
    Vector3 roomPosition(round(position.x_ / 11.0f) * 11.0f, position.y_, round(position.z_ / 11.0f) * 11.0f);
    Vector3 roomOffset = position - roomPosition;
    //LOGERRORF("Offset: %s", roomOffset.ToString().CString());

    bool doorHorizontal = abs(roomOffset.x_) > abs(roomOffset.z_);
    bool travelingUpRight = (doorHorizontal ? roomOffset.x_ : roomOffset.z_) > 0.0f;
    //LOGERRORF("doorHorizontal: %d, travelingUpRight: %d", doorHorizontal, travelingUpRight);

    PODVector<Vector3> path;

    Vector3 doorPosition = node_->GetParent()->GetPosition();
    if (doorHorizontal) {
        if (travelingUpRight) {
            path.Push(doorPosition + Vector3(-1.0f, 0.0f, 0.0f));
            path.Push(doorPosition + Vector3(1.5f, 0.0f, 0.0f));
        } else {
            path.Push(doorPosition + Vector3(1.0f, 0.0f, 0.0f));
            path.Push(doorPosition + Vector3(-1.5f, 0.0f, 0.0f));
        }
    } else {
        if (travelingUpRight) {
            path.Push(doorPosition + Vector3(0.0f, 0.0f, -1.0f));
            path.Push(doorPosition + Vector3(0.0f, 0.0f, 1.5f));
        } else {
            path.Push(doorPosition + Vector3(0.0f, 0.0f, 1.0f));
            path.Push(doorPosition + Vector3(0.0f, 0.0f, -1.5f));
        }
    }

    person->GetComponent<Person>()->SetPath(path);
}

void Door::HandleNodeCollisionEnd(StringHash eventType, VariantMap &eventData)
{
    Node *other = (Node *)eventData[NodeCollisionEnd::P_OTHERNODE].GetPtr();
    Node *person = GetScene()->GetChild("Person", true);
    if (other != person) {
        return;
    }

    node_->SetPosition(Vector3::ZERO);
}

void Door::SetTriggerNode(Node *trigger)
{
    if (trigger_ == trigger) {
        return;
    }

    if (trigger_) {
        UnsubscribeFromEvents(trigger_);
    }
    
    trigger_ = trigger;

    if (trigger_) {
        SubscribeToEvent(trigger_, E_NODECOLLISIONSTART, HANDLER(Door, HandleNodeCollisionStart));
        SubscribeToEvent(trigger_, E_NODECOLLISIONEND, HANDLER(Door, HandleNodeCollisionEnd));
    }
}

Node *Door::GetTriggerNode() const
{
    return trigger_;
}
