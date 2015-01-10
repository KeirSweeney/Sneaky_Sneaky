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
#include "Material.h"
#include "StaticModel.h"
#include "ResourceCache.h"
#include "Renderer.h"
#include "CameraController.h"

using namespace Urho3D;

const float Guard::MOVE_SPEED = 1.0f;
const float Guard::VIEW_DISTANCE = 5.0f;
const float Guard::VIEW_ANGLE = 90.0f;
const float Guard::DETECT_MOVE_SPEED = 1.5f;

Guard::Guard(Context *context):
    LogicComponent(context),
    hasSeenPlayer_(false),
    wasFollowingPlayer_(false)
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
    frontMaterial_ = cache->GetResource<Material>("Materials/MaverickFront.xml");
    backMaterial_ = cache->GetResource<Material>("Materials/MaverickBack.xml");
    leftMaterial_ = cache->GetResource<Material>("Materials/MaverickLeft.xml");
    rightMaterial_ = cache->GetResource<Material>("Materials/MaverickRight.xml");

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
    bool playerDetected = guardMoving && DetectPlayer(personNode);

    if (playerDetected) {
        hasSeenPlayer_ = true;
        FollowPlayer(timeStep, personNode);
    } else {
        FollowWaypoints(timeStep);
    }

    Vector3 velocity = rigidBody_->GetLinearVelocity();
    guardMoving = velocity.LengthSquared() > 0.0f;
    if (!guardMoving) {
        return;
    }

    Quaternion rotation = Quaternion(Vector3::FORWARD, velocity);

    Node *lightNode = node_->GetChild("SearchLight");
    lightNode->SetWorldRotation(rotation);
    lightNode->Rotate(Quaternion(30.0f, Vector3::RIGHT));

    Light *light = lightNode->GetComponent<Light>();
    light->SetColor(guardMoving ? (playerDetected ? Color::RED : Color::WHITE) : Color::BLACK);

    float angle = rotation.YawAngle();

    StaticModel *model = node_->GetComponent<StaticModel>();
    if (angle < -120.0f || angle > 120.0f) {
        model->SetMaterial(frontMaterial_);
    } else if (angle < -60.0f) {
        model->SetMaterial(leftMaterial_);
    } else if (angle > 60.0f) {
        model->SetMaterial(rightMaterial_);
    } else {
        model->SetMaterial(backMaterial_);
    }
}

void Guard::FollowPlayer(float timeStep, Node *player)
{
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

    Vector3 guardPosition = node_->GetWorldPosition();
    Vector3 playerPosition = player->GetWorldPosition();

    wasFollowingPlayer_ = true;

    Vector3 target = navMesh->FindNearestPoint(playerPosition);
    navMesh->FindPath(path_, guardPosition, target);
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

    Vector3 next = path_.Front();

    Vector3 offset = next - position;
    offset.y_ = 0.0f;

    if (offset.LengthSquared() < (MOVE_SPEED * MOVE_SPEED * timeStep * timeStep)) {
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

bool Guard::DetectPlayer(Node *player)
{
    Vector3 guardPosition = node_->GetWorldPosition();
    Vector3 playerPosition = player->GetWorldPosition();
    Vector3 difference = (playerPosition - guardPosition);

    float length = difference.Length();
    if (length > VIEW_DISTANCE) {
        return false;
    }

    Vector3 forward = rigidBody_->GetLinearVelocity();

    forward.Normalize();
    difference.Normalize();

    //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
    //debug->AddLine(guardPosition, guardPosition + forward, Color::BLUE);

    if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
       //debug->AddLine(guardPosition, guardPosition + difference, Color::RED);
       return false;
    }

    //debug->AddLine(guardPosition, guardPosition + difference, Color::GREEN);

    Ray ray(guardPosition + Vector3(0.0f, 1.6f, 0.0f) + (forward * 0.25f), difference);

    PODVector<RayQueryResult> result;
    RayOctreeQuery query(result, ray, RAY_TRIANGLE, length, DRAWABLE_GEOMETRY);

    Octree *octree = GetScene()->GetComponent<Octree>();
    octree->RaycastSingle(query);

    if (!result.Empty() && result[0].node_ != player && result[0].node_->GetParent() != player) {
       //debug->AddLine(ray.origin_, ray.origin_ + (ray.direction_ * length), Color::RED);
       return false;
    }

    //debug->AddLine(ray.origin_, ray.origin_ + (ray.direction_ * length), Color::GREEN);
    return true;
}
