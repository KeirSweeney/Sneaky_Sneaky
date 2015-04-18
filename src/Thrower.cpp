#include "Thrower.h"

#include "Guard.h"
#include "Inventory.h"
#include "Person.h"
#include "Pickup.h"
#include "SelfDestroy.h"

#include "Audio.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Input.h"
#include "Log.h"
#include "Node.h"
#include "ParticleEffect.h"
#include "ParticleEmitter.h"
#include "PhysicsEvents.h"
#include "Profiler.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sound.h"
#include "SoundSource3D.h"

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

void Thrower::Update(float timeStep)
{
	Input *input = GetSubsystem<Input>();

	if(!input->GetKeyPress(KEY_T)) {
		return;
	}

	Inventory *inventory = node_->GetComponent<Inventory>();
	SharedPtr<Pickup> item = inventory->GetItemOfType("Throwable", true);

	if (item.Null()) {
		return;
	}

	item->GetNode()->SetEnabled(true);
	Node *itemNode = item->GetNode();

	ResourceCache *cache = GetSubsystem<ResourceCache>();
	Sound *throwSound = cache->GetResource<Sound>("Audio/HitHurt.wav"); //will need to move this out of update.

	SoundSource3D *source = itemNode->CreateComponent<SoundSource3D>();
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
	itemNode->SubscribeToEvent(itemNode, E_NODECOLLISIONSTART, HANDLER(Thrower, HandleNodeCollision));

	SelfDestroy *selfDestroy = itemNode->CreateComponent<SelfDestroy>();
	selfDestroy->SetLifeTime(7.0f);

	Node *particleEmitterNode = itemNode->CreateChild();
	particleEmitterNode->SetPosition(itemRigidBody->GetCenterOfMass());
	particleEmitterNode->SetScale(Vector3(5.0f, 5.0f, 5.0f) / itemNode->GetScale());

	ParticleEmitter *particleEmitter = particleEmitterNode->CreateComponent<ParticleEmitter>();
	particleEmitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/Trail.xml"));
}

void Thrower::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	Node *itemNode = ((RigidBody *)eventData[NodeCollisionStart::P_BODY].GetPtr())->GetNode();
	SoundSource3D *soundSource = itemNode->GetComponent<SoundSource3D>();

	DistractGuard(itemNode);

	if (soundSource->IsPlaying()) {
		return;
	}

	soundSource->Play(GetSubsystem<ResourceCache>()->GetResource<Sound>("Audio/HitHurt.wav"));
}

void Thrower::DistractGuard(Node *itemNode)
{
	Vector3 itemPos = itemNode->GetWorldPosition();
	PODVector<Node *> guards;
	itemNode->GetScene()->GetChildrenWithComponent<Guard>(guards,true);
	for (PODVector<Node *>::ConstIterator i = guards.Begin(); i != guards.End(); ++i) {
		Node *guardNode = *i;

		Vector3 offset = guardNode->GetWorldPosition() - itemPos;
		if(offset.LengthSquared() > (4.0f * 4.0f)) {
			continue;
		}

		Guard *guard = guardNode->GetComponent<Guard>();
		guard->HeardSound(itemPos);
	}
}
