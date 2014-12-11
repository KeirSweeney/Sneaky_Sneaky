#include "Person.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"

using namespace Urho3D;

Person::Person(Context *context):
    LogicComponent(context),
    target_(Vector3::ZERO)
{
}

void Person::RegisterObject(Context* context)
{
    context->RegisterFactory<Person>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Person::DelayedStart()
{
}

void Person::Update(float timeStep)
{
    PROFILE(PersonUpdate);

    // This should probably be a param.
    NavigationMesh *navMesh = node_->GetScene()->GetComponent<NavigationMesh>();

    Vector3 position = node_->GetWorldPosition();

    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    if (rigidBody->GetLinearVelocity().LengthSquared() < (0.001f * 0.001f)) {
        path_.Clear();
    }

    while (path_.Empty()) {
        navMesh->FindPath(path_, position, navMesh->GetRandomPointInCircle(position, 5.0f));
        path_.Erase(0);
    }

#if 0
    DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
    Vector3 last = position;
    last.y_ += navMesh->GetCellHeight() * 2;
    for (PODVector<Vector3>::ConstIterator i = path_.Begin(); i != path_.End(); ++i) {
        Vector3 next = *i;
        next.y_ += navMesh->GetCellHeight();
        debug->AddLine(last, next, Color(255, 0, 0));
        last = next;
    }
#endif

    //LOGERRORF("Path size: %d", path_.Size());

    target_ = path_.Front();
    target_.y_ -= navMesh->GetCellHeight();

    //LOGERRORF("Position: %f %f %f", position.x_, position.y_, position.z_);
    //LOGERRORF("Target: %f %f %f", target_.x_, target_.y_, target_.z_);

    Vector3 offset = target_ - position;
    if (offset.LengthSquared() < (0.25f * 0.25f)) {
        path_.Erase(0);
    }

    offset.Normalize();
    //node_->SetDirection(offset);
    //node_->Translate(Vector3::FORWARD * 2.0f * timeStep);

    //rigidBody->SetLinearVelocity(offset * 2.0f);
    rigidBody->ApplyImpulse(offset * 2.0f);
}
