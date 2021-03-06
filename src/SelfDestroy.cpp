#include "Urho3D/Urho3D.h"

#include "SelfDestroy.h"

#include "Inventory.h"
#include "Person.h"
#include "Pickup.h"

#include "Urho3D/Audio/Audio.h"
#include "Urho3D/Audio/Sound.h"
#include "Urho3D/Audio/SoundSource3D.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"

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
