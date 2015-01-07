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
#include "Renderer.h"
#include "Material.h"
#include "StaticModel.h"
#include "ResourceCache.h"

using namespace Urho3D;

const float Person::MOVE_SPEED = 2.0f;

const Vector3 Person::DIRECTION_N = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Person::DIRECTION_NE = Vector3(0.707f, 0.0f, 0.707f);
const Vector3 Person::DIRECTION_E = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Person::DIRECTION_SE = Vector3(0.707f, 0.0f, -0.707f);
const Vector3 Person::DIRECTION_S = Vector3(0.0f, 0.0f, -1.0f);
const Vector3 Person::DIRECTION_SW = Vector3(-0.707f, 0.0f, -0.707f);
const Vector3 Person::DIRECTION_W = Vector3(-1.0f, 0.0f, 0.0f);
const Vector3 Person::DIRECTION_NW = Vector3(-0.707f, 0.0f, 0.707f);

Person::Person(Context *context):
    LogicComponent(context),
	moveState_(MS_NONE)
{
}

void Person::RegisterObject(Context* context)
{
    context->RegisterFactory<Person>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Person::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	frontMaterial_ = cache->GetResource<Material>("Materials/MaverickFront.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/MaverickBack.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/MaverickLeft.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/MaverickRight.xml");
}

void Person::Update(float timeStep)
{
    UI *ui = GetSubsystem<UI>();
    Input *input = GetSubsystem<Input>();
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();

    Vector3 position = node_->GetWorldPosition();

    if ((input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonPress(MOUSEB_LEFT)) && !ui->GetElementAt(ui->GetCursorPosition(), false)) {
        Renderer *renderer = GetSubsystem<Renderer>();
        Graphics *graphics = GetSubsystem<Graphics>();

        Camera *camera = renderer->GetViewport(0)->GetCamera();
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
	float distance = offset.LengthSquared();

	if ((path_.Size() == 1 && distance < (0.25f * 0.25f)) || distance < (MOVE_SPEED * MOVE_SPEED * timeStep * timeStep)) {
        path_.Erase(0);
    }

	offset.Normalize();

	Vector3 direction;
	MoveState moveState;
	float angleDot = 0.0f;
	float bestAngleDot = 0.0f;

	if (moveState_ != MS_DIAGONAL && (angleDot = offset.DotProduct(DIRECTION_N)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_N; moveState = MS_CARDINAL; }
	if (moveState_ != MS_CARDINAL && (angleDot = offset.DotProduct(DIRECTION_NE)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_NE; moveState = MS_DIAGONAL; }
	if (moveState_ != MS_DIAGONAL && (angleDot = offset.DotProduct(DIRECTION_E)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_E; moveState = MS_CARDINAL; }
	if (moveState_ != MS_CARDINAL && (angleDot = offset.DotProduct(DIRECTION_SE)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_SE; moveState = MS_DIAGONAL; }
	if (moveState_ != MS_DIAGONAL && (angleDot = offset.DotProduct(DIRECTION_S)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_S; moveState = MS_CARDINAL; }
	if (moveState_ != MS_CARDINAL && (angleDot = offset.DotProduct(DIRECTION_SW)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_SW; moveState = MS_DIAGONAL; }
	if (moveState_ != MS_DIAGONAL && (angleDot = offset.DotProduct(DIRECTION_W)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_W; moveState = MS_CARDINAL; }
	if (moveState_ != MS_CARDINAL && (angleDot = offset.DotProduct(DIRECTION_NW)) > bestAngleDot) { bestAngleDot = angleDot; direction = DIRECTION_NW; moveState = MS_DIAGONAL; }

	if (moveState_ != MS_NONE && direction != lastDirection_) {
		moveState_ = MS_NONE;
		return;
	}

	StaticModel *model = node_->GetComponent<StaticModel>();
	if (direction == DIRECTION_E) {
		model->SetMaterial(leftMaterial_);
	} else if (direction == DIRECTION_W) {
		model->SetMaterial(rightMaterial_);
	} else if (direction == DIRECTION_N || direction == DIRECTION_NE || direction == DIRECTION_NW) {
		model->SetMaterial(backMaterial_);
	} else {
		model->SetMaterial(frontMaterial_);
	}

	moveState_ = moveState;
	lastDirection_ = direction;
	rigidBody->SetLinearVelocity(direction * MOVE_SPEED);
}

void Person::SetTarget(Vector3 target)
{
    path_.Clear();
    path_.Push(target);
}
