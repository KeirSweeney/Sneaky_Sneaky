#include "SelfDestroy.h"

#include "Pickup.h"
#include "Inventory.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Person.h"
#include "Input.h"
#include "StaticModel.h"

using namespace Urho3D;

SelfDestroy::SelfDestroy(Context *context):
    LogicComponent(context),
    decayTime_(3.0f)
{
}

void SelfDestroy::RegisterObject(Context* context)
{
    context->RegisterFactory<SelfDestroy>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void SelfDestroy::DelayedStart()
{
}

void SelfDestroy::Update(float timeStep)
{
    if (lifeTime_ == 0.0f) {
        return;
    }

    if (decayTime_ <= 0) {
        StaticModel *model = node_->GetComponent<StaticModel>();
        model->SetEnabled(!model->IsEnabled());
        decayTime_ = lifeTime_ * 0.1f;
    } else {
        decayTime_ -= timeStep;
    }

    lifeTime_ -= timeStep;

    if (lifeTime_ <= 0.0f) {
        node_->Remove();
    }
}

void SelfDestroy::SetLifeTime(float lifeTime)
{
    lifeTime_ = lifeTime;
}


