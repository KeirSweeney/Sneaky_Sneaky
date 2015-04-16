#pragma once

#include "InteractablePoster.h"

namespace Urho3D {
	class StaticModel;
	class Material;
}

class MrWright : public InteractablePoster
{
	OBJECT(MrWright)

public:
	MrWright(Urho3D::Context *context);
	static void RegisterObject(Urho3D::Context *context);

public:
	void Start();
	void Update(float timeStep);

public:
	void LoadMaterials();

private:
	float swapTimer_;
	Urho3D::Vector<Urho3D::PODVector<Urho3D::Material *>> sequences_;
};
