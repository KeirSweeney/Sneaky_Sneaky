#pragma once

#include "Urho3D/Scene/LogicComponent.h"

namespace Urho3D {
	class Material;
	class UIElement;
	class Text;
}

class Person: public Urho3D::LogicComponent
{
	OBJECT(Person)

private:
	static const float MOVE_SPEED;

public:
	Person(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void DelayedStart();
	void Update(float timeStep);

public:
	void SetPath(Urho3D::PODVector<Urho3D::Vector3> path);
	Urho3D::Vector3 GetDirection() const;
	void ShowHealth();
	void TakeDamage(float damage);

private:
	Urho3D::PODVector<Urho3D::Vector3> path_;
	Urho3D::Material *frontMaterial_;
	Urho3D::Material *frontMaterialAnimated_;
	Urho3D::Material *backMaterial_;
	Urho3D::Material *backMaterialAnimated_;
	Urho3D::Material *leftMaterial_;
	Urho3D::Material *leftMaterialAnimated_;
	Urho3D::Material *rightMaterial_;
	Urho3D::Material *rightMaterialAnimated_;
	Urho3D::Material *frontShadowMaterial_;
	Urho3D::Material *leftShadowMaterial_;
	Urho3D::Vector3 direction_;
	float health_;
	bool showHealth_;
	Urho3D::UIElement *healthBar_;
	Urho3D::Text *healthText_;
};
