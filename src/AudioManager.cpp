#include "Urho3D/Urho3D.h"

#include "AudioManager.h"

#include "Game.h"

#include "Urho3D/Audio/SoundSource.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Physics/PhysicsEvents.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Scene/SceneEvents.h"

using namespace Urho3D;

AudioManager::AudioManager(Context *context):
	Component(context)
{
}

void AudioManager::RegisterObject(Context *context)
{
	context->RegisterFactory<AudioManager>("Logic");

	COPY_BASE_ATTRIBUTES(Component);
}

void AudioManager::OnNodeSet(Node *node)
{
	if (!node) {
		return;
	}

	source_ = node->CreateComponent<SoundSource>();
	source_->SetSoundType(SOUND_VOICE);

	SubscribeToEvent(node, E_SCENEUPDATE, HANDLER(AudioManager, HandleSceneUpdate));
}

void AudioManager::HandleSceneUpdate(StringHash eventType, VariantMap &eventData)
{
	if (source_->IsPlaying()) {
		return;
	}

	if (queue_.Empty()) {
		return;
	}

	AudioQueueEntry &next = queue_.Back();

	if (next.delay > 0.0f) {
		next.delay -= eventData[SceneUpdate::P_TIMESTEP].GetFloat();
		return;
	}

	queue_.Pop();

	if (next.sound) {
		source_->Play(next.sound);
	} else {
		GetSubsystem<Game>()->EndLevel(false, false);
	}
}

void AudioManager::Play(Urho3D::PODVector<AudioQueueEntry> sounds, bool queue)
{
	if (!queue) {
		queue_.Clear();
	}

	queue_.Insert(0, sounds);
}
