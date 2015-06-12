#pragma once

#include "InteractableComponent.h"

namespace Urho3D {

	class RigidBody;
	class StaticModel;
	class SoundSource;
}

class BossLaser :public InteractableComponent
{
	OBJECT(BossLaser);

public:
	BossLaser(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);
	void LoadFromXML(const Urho3D::XMLElement &xml);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);


private:
	Urho3D::RigidBody *rigidBody_;
	Urho3D::StaticModel *model_;
	float laserTime_;
	float laserInterval_;
	float laserDelay_;
	Urho3D::SoundSource *source_;
};