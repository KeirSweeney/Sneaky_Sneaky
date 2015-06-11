#pragma once

#include "AudioManager.h"

#include "Urho3D/Scene/Component.h"

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

public:
	void EnqueueAudioClip(Urho3D::Sound *sound, float delay = 0.0f);

private:
	Urho3D::PODVector<AudioQueueEntry> queue_;
};
