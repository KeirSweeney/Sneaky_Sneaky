#include "Guard.h"


#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Input.h"
#include "Camera.h"
#include "Graphics.h"
#include "Octree.h"
#include "UI.h"

using namespace Urho3D;

Guard::Guard(Context *context):LogicComponent(context)
{
}

void Guard::RegisterObject(Context* context)
{
    context->RegisterFactory<Guard>("Logic");
    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Guard::Update(float timeStep)
{
    UI *ui = GetSubsystem<UI>();
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    Vector3 position = node_->GetWorldPosition();

    if(path_.Empty())
    {
        path_ = waypoints_;
        return;
    }


    Vector3 next = path_.Front();

    Vector3 offset = next - position;
    offset.y_ = 0.0f;

    if (offset.LengthSquared() < (2.0f * 2.0f * timeStep * timeStep)) {
        path_.Erase(0);
    }

    offset.Normalize();
    node_->SetDirection(offset);
    rigidBody->SetLinearVelocity(offset * 2.0f);

}


void Guard::SetWaypoints(PODVector<Vector3>  &waypoints)
{

    path_ = waypoints_ = waypoints;


}

