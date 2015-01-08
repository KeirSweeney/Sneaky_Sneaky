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
#include "Light.h"
#include "Game.h"

using namespace Urho3D;

const float Guard::MOVE_SPEED = 1.0f;
const float Guard::VIEW_DISTANCE = 5.0f;
const float Guard::VIEW_ANGLE = 90.0f;
const float Guard::DETECT_MOVE_SPEED = 1.5f;

Guard::Guard(Context *context):
    LogicComponent(context),
    wasFollowingPlayer_(false)
{
}

void Guard::RegisterObject(Context* context)
{
    context->RegisterFactory<Guard>("Logic");
    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Guard::Update(float timeStep)
{
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    Node *personNode = GetScene()->GetChild("Person", true);

    PODVector<RigidBody *> colliders;
    rigidBody->GetCollidingBodies(colliders);

    for (PODVector<RigidBody *>::ConstIterator i = colliders.Begin(); i != colliders.End(); ++i) {
        if ((*i)->GetNode() != personNode) {
            continue;
        }

        GetSubsystem<Game>()->EndLevel(true);
        return;
    }

    bool playerDetected = DetectPlayer(personNode);

    Light *light = node_->GetChild((unsigned)0)->GetComponent<Light>();
    light->SetColor(playerDetected ? Color::RED : Color::WHITE); //this is too beautiful for me to handle.....

    if (!playerDetected) {
        FollowWaypoints(timeStep);
    } else {
        hasSeenPlayer_ = true;
        FollowPlayer(timeStep, personNode);
    }
}

void Guard::FollowPlayer(float timeStep, Node *player)
{
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

    RigidBody *rigidBody = node_->GetComponent<RigidBody>();

    Vector3 guardPosition = node_->GetWorldPosition();
    Vector3 playerPosition = player->GetWorldPosition();

    wasFollowingPlayer_ = true;

    Vector3 target = navMesh->FindNearestPoint(playerPosition);
    navMesh->FindPath(path_, guardPosition, target);
    path_.Erase(0);

    if (path_.Empty()) {
        rigidBody->SetLinearVelocity(Vector3::ZERO);
        return;
    }

    Vector3 next = path_.Front();

    Vector3 offset = next - guardPosition;
    offset.y_ = 0.0f;

	if (offset.LengthSquared() < (DETECT_MOVE_SPEED * DETECT_MOVE_SPEED * timeStep * timeStep)) {
        path_.Erase(0);
    }

    offset.Normalize();
    node_->SetDirection(offset);
	rigidBody->SetLinearVelocity(offset * DETECT_MOVE_SPEED);
}

void Guard::FollowWaypoints(float timeStep)
{
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
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

    Vector3 next = path_.Front();

    Vector3 offset = next - position;
    offset.y_ = 0.0f;

    if (offset.LengthSquared() < (MOVE_SPEED * MOVE_SPEED * timeStep * timeStep)) {
        path_.Erase(0);
    }

    offset.Normalize();
    node_->SetDirection(offset);
    rigidBody->SetLinearVelocity(offset * MOVE_SPEED);
}

void Guard::SetWaypoints(PODVector<Vector3> &waypoints)
{
    path_ = waypoints_ = waypoints;
}

bool Guard::HasSeenPlayer()
{
    return hasSeenPlayer_;
}

bool Guard::DetectPlayer(Node *player)
{
    Vector3 guardPosition = node_->GetWorldPosition();
    Vector3 playerPosition = player->GetWorldPosition();
    Vector3 difference = (playerPosition - guardPosition);

   if (difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE)) {
        return false;
   }

   Vector3 forward = node_->GetWorldDirection();
   difference.Normalize();

   //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
   //debug->AddLine(guardPosition, guardPosition + forward, Color::BLUE);
   //debug->AddLine(guardPosition, guardPosition + difference, Color::RED);

   if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
       return false;
   }

   Ray ray(guardPosition + Vector3(0.0f, 1.6f, 0.0f) + (forward * 0.25f), difference);
   //debug->AddLine(ray.origin_, ray.origin_ + (ray.direction_ * VIEW_DISTANCE), Color::WHITE);

   PODVector<RayQueryResult> result;
   RayOctreeQuery query(result, ray, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);

   Octree *octree = GetScene()->GetComponent<Octree>();
   octree->RaycastSingle(query);

   if (result.Empty() || result[0].node_ != player) {
       return false;
   }

   return true;
}
