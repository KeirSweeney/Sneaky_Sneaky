#include "SecurityCamera.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "Log.h"

using namespace Urho3D;

SecurityCamera::SecurityCamera(Context *context) :
    InteractableComponent(context),
    sweepingBack_(false),
    pitch_(-15.0f), yaw_(0.0f)
{
}

void SecurityCamera::RegisterObject(Context* context)
{
	context->RegisterFactory<SecurityCamera>("Logic");
    COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void SecurityCamera::DelayedStart()
{
    startRotation_ = node_->GetWorldRotation();
}

void SecurityCamera::Update(float timeStep)
{
    yaw_ += (sweepingBack_ ? -1.0f : 1.0f) * 20.0f * timeStep;

    if (abs(yaw_) >= 45.0f) {
        sweepingBack_ = !sweepingBack_;
    }

    node_->SetWorldRotation(startRotation_);
    node_->Rotate(Quaternion(pitch_, Vector3::LEFT));
    node_->Rotate(Quaternion(yaw_, Vector3::UP));
}
