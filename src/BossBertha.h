#pragma once

#include "InteractableComponent.h"

namespace Urho3D {
	class StaticModel;
	class RigidBody;
	class Material;
	class UIElement;
	class Text;
}

class BossBertha : public InteractableComponent
{
	OBJECT(BossBertha)

public:
	static const float CHARGE_SPEED;
	static const float CHARGE_INTERVAL;

public:
	BossBertha(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);
	void TakeDamage(float damage);

public:
	void HandleNodeCollisionStart(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

private:
	Urho3D::StaticModel *model_;
	Urho3D::RigidBody *rigidBody_;
	Urho3D::Material *frontMaterial_;
	Urho3D::Material *backMaterial_;
	Urho3D::Material *leftMaterial_;
	Urho3D::Material *rightMaterial_;
	Urho3D::Vector3 target_;
	float chargeTimer_;
	float health_;
	Urho3D::UIElement *healthBar_;
	Urho3D::Text *healthText_;
};
