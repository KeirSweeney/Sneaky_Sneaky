#include "Guard.h"

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
}

void Guard::Update(float timeStep)
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

	PODVector<RayQueryResult> result; //stores all of the objects the ray collides with into a vector.
	RayOctreeQuery query(result, ray, RAY_TRIANGLE, length, DRAWABLE_GEOMETRY);
	//the query here checks the result vector at the ray cast from the guards at length.
	// And checks all of the triangles of the objects and only hits drawble geometry.

	Octree *octree = GetScene()->GetComponent<Octree>();
	octree->RaycastSingle(query);//we apply our Ray query here.

	if (!result.Empty() && result[0].node_ != player && result[0].node_->GetParent() != player) {
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
