#include "Thrower.h"
#include "Pickup.h"
#include "Inventory.h"
#include "SelfDestroy.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Person.h"
#include "Input.h"


using namespace Urho3D;


Thrower::Thrower(Context *context):
    LogicComponent(context)
{
}

void Thrower::RegisterObject(Context* context)
{
    context->RegisterFactory<Thrower>("Logic");
    SelfDestroy::RegisterObject(context);

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Thrower::DelayedStart()
{
}

void Thrower::Update(float timeStep)
{
    Input *input = GetSubsystem<Input>();

    if(!input->GetKeyPress(KEY_T))
    {
        return;
    }

    Inventory *inv = node_->GetComponent<Inventory>();
    SharedPtr<Pickup> item =  inv->GetThrowableItem();

    item->GetNode()->SetEnabled(true);

    Node *itemNode = item->GetNode();
    Node *playerNode = GetScene()->GetChild("Person", true);

    RigidBody *itemRigidBody = itemNode->GetComponent<RigidBody>();
    RigidBody *playerRigidBody = playerNode->GetComponent<RigidBody>();

    itemNode->SetWorldPosition(playerNode->GetWorldPosition()+(playerRigidBody->GetLinearVelocity() * 0.5f) + Vector3(0.0f, 1.6f, 0.0f));
    item->SetEnabled(false);
    itemRigidBody->SetTrigger(false);
    itemRigidBody->SetMass(1.0f);
    itemRigidBody->SetRestitution(1.0f);
    itemRigidBody->SetLinearVelocity((playerRigidBody->GetLinearVelocity() * 4.0f) + Vector3(0.0f, 1.6f, 0.0f));

    SelfDestroy *selfDest = itemNode->CreateComponent<SelfDestroy>();
    selfDest->SetLifeTime(7.0f);



}


