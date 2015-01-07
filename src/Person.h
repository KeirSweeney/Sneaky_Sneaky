#pragma once

#include "LogicComponent.h"

namespace Urho3D {
	class Material;
}

class Person: public Urho3D::LogicComponent
{
    OBJECT(Person)

private:
    static const float MOVE_SPEED;

	// Probably going to need to move these to a utility class somewhere.
	static const Urho3D::Vector3 DIRECTION_N;
	static const Urho3D::Vector3 DIRECTION_NE;
	static const Urho3D::Vector3 DIRECTION_E;
	static const Urho3D::Vector3 DIRECTION_SE;
	static const Urho3D::Vector3 DIRECTION_S;
	static const Urho3D::Vector3 DIRECTION_SW;
	static const Urho3D::Vector3 DIRECTION_W;
	static const Urho3D::Vector3 DIRECTION_NW;

public:
    Person(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context* context);
    
public:
    void DelayedStart();
    void Update(float timeStep);

public:
    void SetTarget(Urho3D::Vector3 target);

private:
    Urho3D::PODVector<Urho3D::Vector3> path_;
	Urho3D::Vector3 lastDirection_;
	enum MoveState {
		MS_NONE,
		MS_CARDINAL,
		MS_DIAGONAL
	} moveState_;

	Urho3D::Material *frontMaterial_;
	Urho3D::Material *backMaterial_;
	Urho3D::Material *leftMaterial_;
	Urho3D::Material *rightMaterial_;
};
