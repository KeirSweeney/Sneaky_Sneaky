#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class RigidBody;
	class StaticModel;
	class Material;
}

class BossBertha : public InteractableComponent
{
	OBJECT(BossBertha)

public:
	static const float CHARGE_SPEED;
	static const float VIEW_DISTANCE;
	static const float VIEW_ANGLE;


public:
	BossBertha(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);

private:
	bool DetectPlayer(Urho3D::Node *player);
	void ChargeToPlayer(float timeStep, Urho3D::Node *player);

public:
	void HandleNodeCollision(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
	bool HasSeenPlayer();

private:
	Urho3D::RigidBody *rigidBody_;
	bool hasSeenPlayer_;
	Urho3D::Material *frontMaterial_;
	Urho3D::Material *backMaterial_;
	Urho3D::Material *leftMaterial_;
	Urho3D::Material *rightMaterial_;
};
