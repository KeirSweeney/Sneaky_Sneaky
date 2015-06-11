#include "Urho3D/Urho3D.h"

#include "Person.h"

#include "CameraController.h"
#include "ClickMarker.h"
#include "Game.h"
#include "Wife.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"

using namespace Urho3D;

const float Person::MOVE_SPEED = 2.0f;

Person::Person(Context *context):
	LogicComponent(context),
	health_(1.0),
	showHealth_(false),
	healthBar_(NULL),
	healthText_(NULL)
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
	frontMaterial_ = cache->GetResource<Material>("Materials/MaverickForward.xml");
	frontMaterialAnimated_ = cache->GetResource<Material>("Materials/MaverickForwardAnimated.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/MaverickBack.xml");
	backMaterialAnimated_ = cache->GetResource<Material>("Materials/MaverickBackAnimated.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/MaverickLeft.xml");
	leftMaterialAnimated_ = cache->GetResource<Material>("Materials/MaverickLeftAnimated.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/MaverickRight.xml");
	rightMaterialAnimated_ = cache->GetResource<Material>("Materials/MaverickRightAnimated.xml");
	frontShadowMaterial_ = cache->GetResource<Material>("Materials/MaverickForwardShadow.xml");
	leftShadowMaterial_ = cache->GetResource<Material>("Materials/MaverickLeftShadow.xml");
}

void Person::Update(float timeStep)
{
	UI *ui = GetSubsystem<UI>();

	if (showHealth_ && !healthBar_) {
		healthBar_ = ui->GetRoot()->CreateChild<UIElement>();
		healthBar_->SetFixedSize(healthBar_->GetParent()->GetWidth(), 25);
		healthBar_->SetVerticalAlignment(VA_TOP);
		healthBar_->SetVisible(true);

		Sprite *background = healthBar_->CreateChild<Sprite>();
		background->SetFixedSize(healthBar_->GetSize());
		background->SetColor(Color::RED);
		background->SetOpacity(0.75f);

		healthText_ = healthBar_->CreateChild<Text>();
		healthText_->SetFont("Fonts/Anonymous Pro.ttf");
		healthText_->SetColor(Color::WHITE);
		healthText_->SetText("Player Health: UNKNOWN");
		healthText_->SetAlignment(HA_CENTER, VA_CENTER);
		healthText_->SetTextAlignment(HA_CENTER);
	} else if (healthBar_ && !showHealth_) {
		healthBar_->Remove();
		healthBar_ = NULL;
	}

	if (healthBar_) {
		healthText_->SetText("Player Health: " + String(round(health_ * 100.0f)) + "%");
	}

	Input *input = GetSubsystem<Input>();
	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
	RigidBody *rigidBody = node_->GetComponent<RigidBody>();

	Vector3 position = node_->GetWorldPosition();

	IntVector2 room((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	bool roomDark = GetScene()->GetChild(ToString("%dx%d", room.x_ + 1, room.y_ + 1))->GetVar("dark").GetBool();

	Node *searchLight = node_->GetChild("SearchLight");
	searchLight->SetEnabled(roomDark);

	Quaternion lightRotation(Vector3::FORWARD, direction_);
	lightRotation = (lightRotation * Quaternion(30.0f, Vector3::RIGHT)).Normalized();
	lightRotation = searchLight->GetWorldRotation().Slerp(lightRotation, 5.0f * timeStep);

	searchLight->SetWorldRotation(lightRotation);

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	if (input->IsMouseVisible() || (input->GetNumTouches() > 0 && !input->GetTouch(0)->GetTouchedElement())) {
		Graphics *graphics = GetSubsystem<Graphics>();

		IntVector2 mousePosition = input->IsMouseVisible() ? input->GetMousePosition() : input->GetTouch(0)->position_;
		Ray mouseRay = camera->GetScreenRay(mousePosition.x_ / (float)graphics->GetWidth(), mousePosition.y_ / (float)graphics->GetHeight());

		Octree *octree = GetScene()->GetComponent<Octree>();

		PODVector<RayQueryResult> result;
		RayOctreeQuery query(result, mouseRay, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY, 0x01);
		octree->RaycastSingle(query);

		Vector3 target = position;
		if (!result.Empty()) {
			//LOGERRORF("result[0].position_: %s", result[0].position_.ToString().CString());
			target = result[0].position_;
			target.y_ = 0.0f;
		}

		if (!input->IsMouseVisible() || input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonPress(MOUSEB_LEFT)) {
			Vector3 meshTarget = navMesh->FindNearestPoint(target, Vector3(1.0f, 0.01f, 1.0f));

			if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
				Node *markerNode = GetScene()->CreateChild();
				markerNode->SetPosition(meshTarget);
				markerNode->CreateComponent<ClickMarker>();
			}

			navMesh->FindPath(path_, position, meshTarget);
			path_.Erase(0);
		}

		direction_ = (path_.Empty() ? target : path_.Front()) - position;
		direction_.y_ = 0.0f;
		direction_.Normalize();
	}

	float angle = Quaternion(node_->GetDirection(), direction_).YawAngle();

	StaticModel *model = node_->GetComponent<StaticModel>();
	StaticModel *shadowModel = node_->GetChild("ShadowCaster")->GetComponent<StaticModel>();
	if (angle < -120.0f || angle > 120.0f) {
		model->SetMaterial(path_.Empty() ? frontMaterial_ : frontMaterialAnimated_);
		shadowModel->SetMaterial(leftShadowMaterial_);
	} else if (angle < -60.0f) {
		model->SetMaterial(path_.Empty() ? leftMaterial_ : leftMaterialAnimated_);
		shadowModel->SetMaterial(frontShadowMaterial_);
	} else if (angle > 60.0f) {
		model->SetMaterial(path_.Empty() ? rightMaterial_ : rightMaterialAnimated_);
		shadowModel->SetMaterial(frontShadowMaterial_);
	} else {
		model->SetMaterial(path_.Empty() ? backMaterial_ : backMaterialAnimated_);
		shadowModel->SetMaterial(leftShadowMaterial_);
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

	rigidBody->SetLinearVelocity(offset * MOVE_SPEED);
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

void Person::ShowHealth()
{
	showHealth_ = true;
}

void Person::TakeDamage(float damage)
{
	if (health_ <= 0.0f) {
		return;
	}

	health_ -= damage;

	if (health_ <= 0.0f) {
		health_ = 0.0f;

		GetSubsystem<Game>()->EndLevel(true, false);
	}
}
