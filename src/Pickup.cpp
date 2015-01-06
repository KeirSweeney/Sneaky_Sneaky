#include "Pickup.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "ValueAnimation.h"

using namespace Urho3D;

Pickup::Pickup(Context *context):
    LogicComponent(context),
    height_(0.0f)
{
}

void Pickup::RegisterObject(Context* context)
{
    context->RegisterFactory<Pickup>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
    ATTRIBUTE("Height", float, height_, 0.0f, AM_DEFAULT);
}

void Pickup::DelayedStart()
{
    node_->GetComponent<RigidBody>()->SetTrigger(true);

    SharedPtr<ValueAnimation> animation(new ValueAnimation(context_));
    animation->SetInterpolationMethod(IM_SPLINE);
    animation->SetSplineTension(0.0f);
    animation->SetKeyFrame(0.0f, HEIGHT_MIN);
    animation->SetKeyFrame(1.0f, HEIGHT_MAX);
    animation->SetKeyFrame(2.0f, HEIGHT_MIN);
    SetAttributeAnimation("Height", animation);
}

void Pickup::Update(float timeStep)
{
    node_->Rotate(Quaternion(ROTATION_SPEED * timeStep, Vector3::UP));

    Vector3 position = node_->GetPosition();
    position.y_ = height_;
    node_->SetPosition(position);
}
