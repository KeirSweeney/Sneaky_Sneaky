#include "BossBertha.h"

#include "CameraController.h"
#include "Game.h"

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

using namespace Urho3D;

const float BossBertha::CHARGE_SPEED = 10.0f;
const float BossBertha::VIEW_DISTANCE = 10.0f;
const float BossBertha::VIEW_ANGLE = 360.0f;

BossBertha::BossBertha(Context *context) :
InteractableComponent(context),
hasSeenPlayer_(false)
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

	rigidBody_ = node_->GetComponent<RigidBody>();



	rigidBody_->SetMass(100.0f);
	rigidBody_->SetFriction(0.0f);
	rigidBody_->SetAngularFactor(Vector3::ZERO);
}


void BossBertha::Update(float timeStep)
{

	Node *personNode = GetScene()->GetChild("Person", true);
	PODVector<RigidBody *> colliders;
	rigidBody_->GetCollidingBodies(colliders);

	for (PODVector<RigidBody *>::ConstIterator i = colliders.Begin(); i != colliders.End(); ++i) {
		if ((*i)->GetNode() != personNode) {
			continue;
		}

		GetSubsystem<Game>()->EndLevel(true);
		return;
	}

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	bool BerthaCharging = rigidBody_->GetLinearVelocity().LengthSquared() > 0.0f;

	Vector3 velocity = rigidBody_->GetLinearVelocity();
	BerthaCharging = velocity.LengthSquared() > 0.0f;

	bool playerDetected = DetectPlayer(personNode);

	if (playerDetected){
		hasSeenPlayer_ = true;
		//ChargeToPlayer(timeStep, personNode);
	}

	ChargeToPlayer(timeStep, personNode);

	Vector3 position = node_->GetWorldPosition();
	IntVector2 room((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	bool roomDark = GetScene()->GetChild(ToString("%dx%d", room.x_ + 1, room.y_ + 1))->GetVar("dark").GetBool();

	Quaternion rotation = Quaternion(Vector3::FORWARD, velocity);

	StaticModel *model = node_->GetComponent<StaticModel>();
	float angle = rotation.YawAngle();

	if (angle < -120.0f || angle > 120.0f) {
		model->SetMaterial(frontMaterial_);
	}
	else if (angle < -60.0f) {
		model->SetMaterial(leftMaterial_);
	}
	else if (angle > 60.0f) {
		model->SetMaterial(rightMaterial_);
	}
	else {
		model->SetMaterial(backMaterial_);
	}
	

}

void BossBertha::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
}


bool BossBertha::DetectPlayer(Node *player)
{
	Vector3 cameraPosition = node_->GetWorldPosition();
	Vector3 cameraDirection = node_->GetWorldDirection();
	Vector3 playerPosition = player->GetWorldPosition();

	Vector3 difference = (playerPosition - cameraPosition);
	difference.Normalize();

	if (cameraDirection.DotProduct(difference) < Cos(360.0f))
		return false;


	return true;
}

void BossBertha::ChargeToPlayer(float timeStep, Node *player)
{

	Vector3 berthaPosition = node_->GetWorldPosition();
	Vector3 playerPosition = player->GetWorldPosition();

	Vector3 difference = playerPosition - berthaPosition;

	difference.Normalize();
	rigidBody_->SetLinearVelocity(difference * 2.4f);
	LOGERRORF("Linear Velocity: %s", rigidBody_->GetLinearVelocity().ToString().CString());
}




