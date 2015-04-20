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
