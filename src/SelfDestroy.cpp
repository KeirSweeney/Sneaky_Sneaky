#include "SelfDestroy.h"

#include "Inventory.h"
#include "Person.h"
#include "Pickup.h"

#include "Audio.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Input.h"
#include "Log.h"
#include "Node.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sound.h"
#include "SoundSource3D.h"
#include "StaticModel.h"

using namespace Urho3D;

SelfDestroy::SelfDestroy(Context *context):
	LogicComponent(context),
	decayTime_(3.0f)
{
}

void SelfDestroy::RegisterObject(Context* context)
{
	context->RegisterFactory<SelfDestroy>("Logic");

	COPY_BASE_ATTRIBUTES(LogicComponent);
}

void SelfDestroy::DelayedStart()
{
}

void SelfDestroy::Update(float timeStep)
{
	if (lifeTime_ == 0.0f) {
		return;
	}

	if (decayTime_ <= 0) {
		StaticModel *model = node_->GetComponent<StaticModel>();
		model->SetEnabled(!model->IsEnabled());
		decayTime_ = lifeTime_ * 0.1f;
	} else {
		decayTime_ -= timeStep;
	}

	lifeTime_ -= timeStep;

	if (lifeTime_ <= 0.0f) {
		ResourceCache *cache = GetSubsystem<ResourceCache>();
		Sound *destroySound = cache->GetResource<Sound>("Audio/Explosion.wav"); //will need to move this out of update.
		Node *soundNode = node_->GetScene()->CreateChild();
		soundNode->SetWorldPosition(node_->GetWorldPosition());
		SoundSource3D *source = soundNode->CreateComponent<SoundSource3D>();
		source->SetAutoRemove(true);

		source->Play(destroySound);
		node_->Remove(); //not working, maybe because the node is destroyed before it can make the sound or the node_createcomponent is wrong as it already has the component?
	}
}

void SelfDestroy::SetLifeTime(float lifeTime)
{
	lifeTime_ = lifeTime;
}
