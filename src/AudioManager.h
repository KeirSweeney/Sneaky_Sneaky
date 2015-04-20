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

class AudioManager: public Urho3D::Component
{
	OBJECT(AudioManager)

public:
	AudioManager(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void OnNodeSet(Urho3D::Node *node);

public:
	void HandleSceneUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

public:
	void Play(Urho3D::PODVector<AudioQueueEntry> sounds, bool queue = false);

private:
	Urho3D::SoundSource *source_;
	Urho3D::PODVector<AudioQueueEntry> queue_;
};
