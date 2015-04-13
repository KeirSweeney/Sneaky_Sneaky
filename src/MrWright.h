#pragma once

#include "InteractablePoster.h"

namespace Urho3D {
    class StaticModel;
}

class MrWright : public InteractablePoster
{
    OBJECT(MrWright)



public:
    MrWright(Urho3D::Context *context);
    static void RegisterObject(Urho3D::Context *context);

    
public:
	void Start();
    void DelayedStart();
    void Update(float timeStep);



private:
    Urho3D::String content_;
	Urho3D::SharedPtr<Urho3D::StaticModel> glyphs_[6];
	float swapTimer_;

};
