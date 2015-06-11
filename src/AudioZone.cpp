#include "Urho3D/Urho3D.h"

#include "AudioZone.h"

#include "Urho3D/Audio/SoundSource.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Physics/PhysicsEvents.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Scene/SceneEvents.h"

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

void AudioZone::EnqueueAudioClip(Sound *sound, float delay)
{
	queue_.Insert(0, {sound, delay});
}

void AudioZone::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();
	Node *person = GetScene()->GetChild("Person", true);
	if (other != person) {
		return;
	}

	GetScene()->GetComponent<AudioManager>()->Play(queue_);

	node_->Remove();
}
