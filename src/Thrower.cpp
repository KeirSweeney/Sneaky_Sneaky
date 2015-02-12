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
#include "ParticleEmitter.h"
#include "ParticleEffect.h"
#include "ResourceCache.h"
#include "Audio.h"
#include "SoundSource.h"
#include "SoundSource3D.h"
#include "Sound.h"

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

void Thrower::Start()
{
    //SubscribeToEvent(E_NODECOLLSION,HANDLER(Thrower,HandleNodeCollision));
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
    SharedPtr<Pickup> item = inv->GetThrowableItem();

    if (item.Null()) {
        return;
    }

    item->GetNode()->SetEnabled(true);
    Node *itemNode = item->GetNode();

    ResourceCache *cache = GetSubsystem<ResourceCache>();
    Sound *throwSound = cache->GetResource<Sound>("Audio/HitHurt.wav"); //will need to move this out of update.

    SoundSource *source = itemNode->CreateComponent<SoundSource>();
    source->SetAutoRemove(true);
    source->Play(throwSound);

    RigidBody *itemRigidBody = itemNode->GetComponent<RigidBody>();    
    Person *person = node_->GetComponent<Person>();
    Vector3 personDirection = person->GetDirection();

    itemNode->SetWorldPosition(node_->GetWorldPosition()+(personDirection * 0.5f) + Vector3(0.0f, 1.6f, 0.0f));
    item->SetEnabled(false);
    itemRigidBody->SetTrigger(false);
    itemRigidBody->SetMass(1.0f);
    itemRigidBody->SetRestitution(1.0f);
    itemRigidBody->SetLinearVelocity((personDirection * 4.0f) + Vector3(0.0f, 1.6f, 0.0f));



    ParticleEmitter *particleEmitter = itemNode->CreateComponent<ParticleEmitter>();
    particleEmitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/Trail.xml"));

    SelfDestroy *selfDestroy = itemNode->CreateComponent<SelfDestroy>();
    selfDestroy->SetLifeTime(7.0f);
}

/*void Thrower::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{


}*/
