#pragma once

#include "Component.h"

namespace Urho3D {
	class Sound;
	class SoundSource;
}

struct AudioQueueEntry {
	Urho3D::Sound *sound;
	float delay;
};

class AudioZone: public Urho3D::Component
{
	OBJECT(AudioZone)

public:
	AudioZone(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void OnNodeSet(Urho3D::Node *node);

public:
	void HandleNodeCollisionStart(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
	void HandleSceneUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

public:
	void EnqueueAudioClip(Urho3D::Sound *sound, float delay = 0.0f);

private:
	Urho3D::SoundSource *source_;
	Urho3D::PODVector<AudioQueueEntry> queue_;
};
