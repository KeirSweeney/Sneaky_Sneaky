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
#include "UI.h"

using namespace Urho3D;

Person::Person(Context *context):
    LogicComponent(context)
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
    UI *ui = GetSubsystem<UI>();
    Input *input = GetSubsystem<Input>();
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();

    Vector3 position = node_->GetWorldPosition();

    if (input->GetMouseButtonPress(MOUSEB_LEFT) && !ui->GetElementAt(ui->GetCursorPosition(), false)) {
        Camera *camera = GetScene()->GetChild("Camera", true)->GetComponent<Camera>();
        Graphics *graphics = GetSubsystem<Graphics>();

        IntVector2 mousePosition = input->GetMousePosition();
        Ray mouseRay = camera->GetScreenRay(mousePosition.x_ / (float)graphics->GetWidth(), mousePosition.y_ / (float)graphics->GetHeight());

        Octree *octree = GetScene()->GetComponent<Octree>();

        PODVector<RayQueryResult> result;
        RayOctreeQuery query(result, mouseRay, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);
        octree->RaycastSingle(query);

        if (!result.Empty()) {
            //LOGERRORF("result[0].position_: %s", result[0].position_.ToString().CString());
            Vector3 target = navMesh->FindNearestPoint(result[0].position_);
            navMesh->FindPath(path_, position, target);
            path_.Erase(0);
        }
    }

    if (path_.Empty()) {
        rigidBody->SetLinearVelocity(Vector3::ZERO);
        return;
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

void Person::SetTarget(Vector3 target)
{
    path_.Clear();
    path_.Push(target);
}
