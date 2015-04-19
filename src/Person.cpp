#include "Person.h"

#include "CameraController.h"
#include "ClickMarker.h"

#include "Camera.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Graphics.h"
#include "Input.h"
#include "Log.h"
#include "Material.h"
#include "NavigationMesh.h"
#include "Node.h"
#include "Octree.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "StaticModel.h"
#include "UI.h"

using namespace Urho3D;

const float Person::MOVE_SPEED = 2.0f;

Person::Person(Context *context):
	LogicComponent(context),
	health_(3)
{
}

void Person::RegisterObject(Context* context)
{
	context->RegisterFactory<Person>("Logic");

	ClickMarker::RegisterObject(context);

	COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Person::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	frontMaterial_ = cache->GetResource<Material>("Materials/MaverickFront.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/MaverickBack.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/MaverickLeft.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/MaverickRight.xml");
	frontShadowMaterial_ = cache->GetResource<Material>("Materials/MaverickFrontShadow.xml");
	leftShadowMaterial_ = cache->GetResource<Material>("Materials/MaverickLeftShadow.xml");
}

void Person::Update(float timeStep)
{
	UI *ui = GetSubsystem<UI>();
	Input *input = GetSubsystem<Input>();
	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
	RigidBody *rigidBody = node_->GetComponent<RigidBody>();

	Vector3 position = node_->GetWorldPosition();

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	if ((input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonPress(MOUSEB_LEFT)) && !ui->GetElementAt(ui->GetCursorPosition(), false)) {
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
			target.y_ = 0.0f;

			if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
				Node *markerNode = GetScene()->CreateChild();
				markerNode->SetPosition(target);
				markerNode->CreateComponent<ClickMarker>();
			}

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

	if (distance < (0.25f * 0.25f) || distance < (MOVE_SPEED * MOVE_SPEED * timeStep * timeStep)) {
		path_.Erase(0);
	}

	offset.Normalize();
	float angle = Quaternion(node_->GetDirection(), offset).YawAngle();

	StaticModel *model = node_->GetComponent<StaticModel>();
	StaticModel *shadowModel = node_->GetChild("ShadowCaster")->GetComponent<StaticModel>();
	if (angle < -120.0f || angle > 120.0f) {
		model->SetMaterial(frontMaterial_);
		shadowModel->SetMaterial(leftShadowMaterial_);
	} else if (angle < -60.0f) {
		model->SetMaterial(leftMaterial_);
		shadowModel->SetMaterial(frontShadowMaterial_);
	} else if (angle > 60.0f) {
		model->SetMaterial(rightMaterial_);
		shadowModel->SetMaterial(frontShadowMaterial_);
	} else {
		model->SetMaterial(backMaterial_);
		shadowModel->SetMaterial(leftShadowMaterial_);
	}

	rigidBody->SetLinearVelocity(offset * MOVE_SPEED);
	direction_ = offset;
}

void Person::SetPath(Urho3D::PODVector<Urho3D::Vector3> path)
{
	path_.Clear();
	path_.Push(path);
}

Vector3 Person::GetDirection() const
{
	return direction_;
}

void Person::TakeDamage()
{
	health_ -= 1;
}


