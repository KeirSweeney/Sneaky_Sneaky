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

using namespace Urho3D;

const float Door::DOOR_OFFSET = -1.275f;

Door::Door(Context *context):
    LogicComponent(context)
{
}

void Door::RegisterObject(Context* context)
{
    context->RegisterFactory<Door>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Door::DelayedStart()
{
}

void Door::Update(float timeStep)
{
    if (!trigger_) {
        return;
    }

    RigidBody *rigidBody = trigger_->GetComponent<RigidBody>();

    PODVector<RigidBody *> colliders;
    rigidBody->GetCollidingBodies(colliders);

    Node *person = GetScene()->GetChild("Person", true);

    if (!colliders.Empty() && colliders[0]->GetNode() == person) {
		node_->SetPosition(Vector3(DOOR_OFFSET, 0.0f, 0.0f));

		person->GetComponent<Person>()->SetTarget(person->GetPosition() + (colliders[0]->GetLinearVelocity() * 0.01f));
    } else {
        node_->SetPosition(Vector3::ZERO);
    }
}

void Door::SetTriggerNode(Node *trigger)
{
    trigger_ = trigger;
}

Node *Door::GetTriggerNode() const
{
    return trigger_;
}
