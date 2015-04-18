#include "AudioZone.h"

#include "Context.h"
#include "Node.h"
#include "PhysicsEvents.h"
#include "Scene.h"
#include "SceneEvents.h"
#include "SoundSource.h"

using namespace Urho3D;

AudioZone::AudioZone(Context *context):
	Component(context)
{
}

void AudioZone::RegisterObject(Context *context)
{
	context->RegisterFactory<AudioZone>("Logic");

	COPY_BASE_ATTRIBUTES(Component);
}

void AudioZone::OnNodeSet(Node *node)
{
	if (!node) {
		return;
	}

	SubscribeToEvent(node, E_NODECOLLISIONSTART, HANDLER(AudioZone, HandleNodeCollisionStart));
}

void AudioZone::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
	if (source_->IsPlaying()) {
		return;
	}

	if (queue_.Empty()) {
		node_->Remove();
		return;
	}

	AudioQueueEntry &next = queue_.Back();

	if (next.delay > 0.0f) {
		next.delay -= eventData[SceneUpdate::P_TIMESTEP].GetFloat();
		return;
	}

	source_->SetSoundType(SOUND_VOICE);
	source_->Play(next.sound);

	queue_.Pop();
}

void AudioZone::EnqueueAudioClip(Sound *sound, float delay)
{
	queue_.Insert(0, {sound, delay});
}

void AudioZone::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	UnsubscribeFromEvent(node_, E_NODECOLLISIONSTART);

	source_ = node_->CreateComponent<SoundSource>();

	SubscribeToEvent(GetScene(), E_SCENEUPDATE, HANDLER(AudioZone, HandleSceneUpdate));
}
