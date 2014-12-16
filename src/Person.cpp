#include "Person.h"

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

    Input *input = GetSubsystem<Input>();
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

    Vector3 position = node_->GetWorldPosition();

    if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
        Camera *camera = GetScene()->GetChild("Camera")->GetComponent<Camera>();
        Graphics *graphics = GetSubsystem<Graphics>();

        IntVector2 mousePosition = input->GetMousePosition();
        Ray mouseRay = camera->GetScreenRay(mousePosition.x_ / (float)graphics->GetWidth(), mousePosition.y_ / (float)graphics->GetHeight());

        Octree *octree = GetScene()->GetComponent<Octree>();

        PODVector<RayQueryResult> result;
        RayOctreeQuery query(result, mouseRay, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);
        octree->RaycastSingle(query);

        if (!result.Empty()) {
            //LOGERRORF("result[0].position_: %s", result[0].position_.ToString().CString());
            target_ = navMesh->FindNearestPoint(result[0].position_);
            navMesh->FindPath(path_, position, target_);
            path_.Erase(0);
        }
    }

    if (path_.Empty()) {
        return;
    }

#if 1
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

    Vector3 next = path_.Front();
    next.y_ -= navMesh->GetCellHeight();

    Vector3 offset = next - position;
    if (offset.LengthSquared() < (0.25f * 0.25f)) {
        path_.Erase(0);
    }

    offset.Normalize();

    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    rigidBody->ApplyImpulse(offset * 10.0f);
}
