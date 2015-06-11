#include "Urho3D/Urho3D.h"

#include "Guard.h"

#include "CameraController.h"
#include "Game.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Light.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/PhysicsEvents.h"
#include "Urho3D/Physics/PhysicsWorld.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"

using namespace Urho3D;

const float Guard::MOVE_SPEED = 1.0f;
const float Guard::VIEW_DISTANCE = 5.0f;
const float Guard::VIEW_ANGLE = 75.0f;
const float Guard::DETECT_MOVE_SPEED = 1.2f;

Guard::Guard(Context *context):
	LogicComponent(context),
	hasSeenPlayer_(false),
	wasFollowingPlayer_(false),
	willHearSound_(false)
{
}

void Guard::RegisterObject(Context* context)
{
	context->RegisterFactory<Guard>("Logic");
	COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Guard::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	frontMaterial_ = cache->GetResource<Material>("Materials/GuardForward.xml");
	frontMaterialAnimated_ = cache->GetResource<Material>("Materials/GuardForwardAnimated.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/GuardBack.xml");
	backMaterialAnimated_ = cache->GetResource<Material>("Materials/GuardBackAnimated.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/GuardLeft.xml");
	leftMaterialAnimated_ = cache->GetResource<Material>("Materials/GuardLeftAnimated.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/GuardRight.xml");
	rightMaterialAnimated_ = cache->GetResource<Material>("Materials/GuardRightAnimated.xml");

	rigidBody_ = node_->GetComponent<RigidBody>();

	SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(Guard, HandleNodeCollisionStart));
}

void Guard::Update(float timeStep)
{
	Node *personNode = GetScene()->GetChild("Person", true);

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	bool guardMoving = rigidBody_->GetLinearVelocity().LengthSquared() > 0.0f;

	bool playerDetected = false;
	if (guardMoving) {
		if (willHearSound_) {
			wasFollowingPlayer_ = true;

			NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

			Vector3 soundPositionFoor = soundPosition_;
			soundPositionFoor.y_ = 0.0f;

			Vector3 target = navMesh->FindNearestPoint(soundPositionFoor, Vector3(1.0f, 0.01f, 1.0f));
			navMesh->FindPath(path_, node_->GetWorldPosition(), target);
			path_.Erase(0);

			willHearSound_ = false;
		} else {
			playerDetected = DetectPlayer(personNode);
		}
	}

	if (playerDetected) {
		hasSeenPlayer_ = true;
		FollowPlayer(timeStep, personNode);
	} else {
		FollowWaypoints(timeStep);
	}

	Vector3 velocity = rigidBody_->GetLinearVelocity();
	guardMoving = velocity.LengthSquared() > 0.0f;

	Quaternion rotation = Quaternion(node_->GetWorldDirection(), velocity);
	float angle = rotation.YawAngle();

	StaticModel *model = node_->GetComponent<StaticModel>();
	if (angle < -120.0f || angle > 120.0f) {
		model->SetMaterial(guardMoving ? frontMaterialAnimated_ : frontMaterial_);
	} else if (angle < -60.0f) {
		model->SetMaterial(guardMoving ? leftMaterialAnimated_ : leftMaterial_);
	} else if (angle > 60.0f) {
		model->SetMaterial(guardMoving ? rightMaterialAnimated_ : rightMaterial_);
	} else {
		model->SetMaterial(guardMoving ? backMaterialAnimated_ : backMaterial_);
	}

	if (!guardMoving) {
		return;
	}

	Node *lightNode = node_->GetChild("SearchLight");

	Quaternion lightRotation = Quaternion(Vector3::FORWARD, velocity);
	lightRotation = (lightRotation * Quaternion(30.0f, Vector3::RIGHT)).Normalized();
	lightRotation = lightNode->GetWorldRotation().Slerp(lightRotation, 5.0f * timeStep);

	lightNode->SetWorldRotation(lightRotation);

	Light *light = lightNode->GetComponent<Light>();
	light->SetColor(guardMoving ? (playerDetected ? Color::RED : Color::WHITE) : Color::BLACK);
}

void Guard::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();
	Node *personNode = GetScene()->GetChild("Person", true);

	if (other != personNode) {
		return;
	}

	GetSubsystem<Game>()->EndLevel(true, false);
}

void Guard::FollowPlayer(float timeStep, Node *player)
{
	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

	Vector3 guardPosition = node_->GetWorldPosition();
	Vector3 playerPosition = player->GetWorldPosition();

	wasFollowingPlayer_ = true;

	navMesh->FindPath(path_, guardPosition, playerPosition);
	path_.Erase(0);

	if (path_.Empty()) {
		rigidBody_->SetLinearVelocity(Vector3::ZERO);
		return;
	}

	Vector3 next = path_.Front();

	Vector3 offset = next - guardPosition;
	offset.y_ = 0.0f;

	if (offset.LengthSquared() < (DETECT_MOVE_SPEED * DETECT_MOVE_SPEED * timeStep * timeStep)) {
		path_.Erase(0);
	}

	offset.Normalize();
	rigidBody_->SetLinearVelocity(offset * DETECT_MOVE_SPEED);
}

void Guard::FollowWaypoints(float timeStep)
{
	Vector3 position = node_->GetWorldPosition();

	if (wasFollowingPlayer_ && path_.Empty()) {
		NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
		navMesh->FindPath(path_, position, waypoints_[0]);
		path_.Erase(0);

		wasFollowingPlayer_ = false;
	}

	if (path_.Empty()) {
		path_ = waypoints_;
	}

#if 0
	NavigationMesh *navMesh = node_->GetScene()->GetComponent<NavigationMesh>();
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

	if ((wasFollowingPlayer_ && path_.Size() == 1 && offset.LengthSquared() < (1.0f * 1.0f * timeStep * timeStep)) || (offset.LengthSquared() < (MOVE_SPEED * MOVE_SPEED * timeStep * timeStep))) {
		path_.Erase(0);
	}

	offset.Normalize();
	rigidBody_->SetLinearVelocity(offset * MOVE_SPEED);
}

void Guard::SetWaypoints(PODVector<Vector3> &waypoints)
{
	path_ = waypoints_ = waypoints;
}

bool Guard::HasSeenPlayer()
{
	return hasSeenPlayer_;
}

void Guard::HeardSound(Vector3 position)
{
	willHearSound_ = true;
	soundPosition_ = position;
}

bool Guard::DetectPlayer(Node *player)
{
	Vector3 guardPosition = node_->GetWorldPosition();
	Vector3 playerPosition = player->GetWorldPosition();
	Vector3 difference = (playerPosition - guardPosition);

	float length = difference.Length();
	if (length > VIEW_DISTANCE) {
		return false; //if the player is too far away from the guard then return false and exit the function.
	}

	Vector3 forward = rigidBody_->GetLinearVelocity(); //this stores the way that the guard is looking in the game.

	forward.Normalize(); //normalize is used for when we use dot product.
	difference.Normalize();

	//DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	//debug->AddLine(guardPosition, guardPosition + forward, Color::BLUE);

	if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
	   //debug->AddLine(guardPosition, guardPosition + difference, Color::RED);
	   return false; // this if statement is used to see if the player is within the guards FOV.
					 //If he is not then it retruns false and exits the function.
	}

	//debug->AddLine(guardPosition, guardPosition + difference, Color::GREEN);

	Ray ray(guardPosition + Vector3(0.0f, 1.6f, 0.0f) + (forward * 0.25f), difference);
	//this creates a raycast from an increased y value near to his eyes in the forward direction of
	//where he is looking to the difference of positions from the guard to the player.

	PhysicsWorld *physicsWorld = GetScene()->GetComponent<PhysicsWorld>();

	PhysicsRaycastResult result;
	physicsWorld->RaycastSingle(result, ray, VIEW_DISTANCE);

	if (result.body_ && result.body_->GetNode() != player) {
	   //debug->AddLine(ray.origin_, ray.origin_ + (ray.direction_ * length), Color::RED);
	   return false; //If the ray hits anything except the player it returns false and exits the loop.
	}

	//debug->AddLine(ray.origin_, ray.origin_ + (ray.direction_ * length), Color::GREEN);
	return true; //And finnally after all the checks, we know that the guard can see the player, is close enough,
				 //and that there is no object obscuring the guards FOV.
}

void Guard::SetPath(Urho3D::PODVector<Urho3D::Vector3> path)
{
	path_.Clear();
	path_.Push(path);
}
