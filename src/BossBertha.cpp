#include "BossBertha.h"

#include "CameraController.h"
#include "Game.h"
#include "Person.h"

#include "Camera.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Graphics.h"
#include "Input.h"
#include "Light.h"
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
#include "PhysicsEvents.h"
#include "CollisionShape.h"

using namespace Urho3D;

const float BossBertha::CHARGE_SPEED = 4.0f;
const float BossBertha::CHARGE_INTERVAL = 5.0f;

BossBertha::BossBertha(Context *context) :
	InteractableComponent(context),
	chargeTimer_(0.0f)
{
}

void BossBertha::RegisterObject(Context* context)
{
	context->RegisterFactory<BossBertha>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void BossBertha::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	frontMaterial_ = cache->GetResource<Material>("Materials/BerthaFront.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/BerthaBack.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/BerthaLeft.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/BerthaRight.xml");

	model_ = node_->GetComponent<StaticModel>();

	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetMass(100.0f);
	rigidBody_->SetFriction(0.0f);
	rigidBody_->SetAngularFactor(Vector3::ZERO);

	CollisionShape *collisionShape = node_->GetComponent<CollisionShape>();
	collisionShape->SetCylinder(0.5f, 1.8f, Vector3(0.0f, 1.8f / 2.0f, 0.0f));

	SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(BossBertha, HandleNodeCollisionStart));
}


void BossBertha::Update(float timeStep)
{
	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	Vector3 position = node_->GetWorldPosition();

	Node *personNode = GetScene()->GetChild("Person", true);
	Vector3 personPosition = personNode->GetWorldPosition();

	IntVector2 room = IntVector2((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	IntVector2 personRoom = IntVector2((int)round(personPosition.x_ / 11.0f), (int)round(personPosition.z_ / 11.0f));

	if (room != personRoom) {
		return;
	} else {
		Person *person = personNode->GetComponent<Person>();
		person->ShowHealth();
	}

	Vector3 difference = personPosition - position;
	Quaternion rotation = Quaternion(node_->GetWorldDirection(), difference);

	bool isCharging = rigidBody_->GetLinearVelocity().LengthSquared() > 0.1f;

	if (isCharging) {
		// Have her follow the player just a tiny bit.
		target_ = target_.Lerp(personPosition, 1.0f * timeStep);

		Vector3 offset = target_ - position;

		rigidBody_->SetLinearVelocity(offset.Normalized() * CHARGE_SPEED);

		if (offset.LengthSquared() < (CHARGE_SPEED * CHARGE_SPEED * timeStep * timeStep)) {
			rigidBody_->SetLinearVelocity(Vector3::ZERO);

			// Some kind of skid sound?
		}

		return;
	}

	if (Abs(rotation.YawAngle()) >= 90.0f) {
		model_->SetMaterial(frontMaterial_);
	} else {
		model_->SetMaterial(backMaterial_);
	}

	chargeTimer_ += timeStep;

	if (chargeTimer_ < CHARGE_INTERVAL) {
		return;
	}

	chargeTimer_ = 0.0f;

	// Play charge sound here?

	if (rotation.YawAngle() < 0.0f) {
		model_->SetMaterial(leftMaterial_);
	} else {
		model_->SetMaterial(rightMaterial_);
	}

	target_ = personPosition;

	rigidBody_->SetLinearVelocity(difference.Normalized() * CHARGE_SPEED);
}

void BossBertha::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();
	Node *personNode = GetScene()->GetChild("Person", true);

	if (other != personNode) {
		return;
	}

	GetSubsystem<Game>()->EndLevel(true, false);
}
