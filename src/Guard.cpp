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

using namespace Urho3D;

Guard::Guard(Context *context):LogicComponent(context)
{
}

void Guard::RegisterObject(Context* context)
{
    context->RegisterFactory<Guard>("Logic");
    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Guard::Update(float timeStep)
{
    Node *myChar = GetScene()->GetChild("Person",true);

    bool playerDetected = DetectPlayer(myChar);
    Light *light = node_->GetChild((unsigned)0)->GetComponent<Light>();
    light->SetColor(playerDetected ? Color::RED : Color::WHITE);

    if(playerDetected == false)
    {
         FollowWaypoints(timeStep);
    }
    else
    {
        FollowPlayer(timeStep, myChar);
    }





}

void Guard::FollowPlayer(float timeStep,Node *player)
{
    NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();
    Vector3 position = node_->GetWorldPosition();
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    Vector3 playerPos = player->GetWorldPosition();
    Vector3 target = navMesh->FindNearestPoint(playerPos);
    navMesh->FindPath(path_,position, target);
    path_.Erase(0);

    if (path_.Empty()) {
        rigidBody->SetLinearVelocity(Vector3::ZERO);
        return;
    }

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

void Guard::FollowWaypoints(float timeStep)
{
    RigidBody *rigidBody = node_->GetComponent<RigidBody>();
    Vector3 position = node_->GetWorldPosition();

    if(path_.Empty())
    {
        path_ = waypoints_;
        return;
    }


    Vector3 next = path_.Front();

    Vector3 offset = next - position;
    offset.y_ = 0.0f;

    if (offset.LengthSquared() < (1.0f * 1.0f * timeStep * timeStep)) {
        path_.Erase(0);
    }

    offset.Normalize();
    node_->SetDirection(offset);
    rigidBody->SetLinearVelocity(offset * 1.0f);
}

void Guard::SetWaypoints(PODVector<Vector3>  &waypoints)
{
    path_ = waypoints_ = waypoints;
}

bool Guard::DetectPlayer(Node *myChar)
{
    Vector3 nodePos = node_->GetWorldPosition();

    Vector3 charPos = myChar->GetWorldPosition();

    Vector3 charDiff = (charPos - nodePos);

   if(charDiff.LengthSquared() > (5.0f * 5.0f))
   {
        return false;
   }

   Vector3 nodeForward = node_->GetWorldDirection();
   charDiff.Normalize();


   //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
   //->AddLine(nodePos,nodePos+nodeForward,Color::BLUE);
   //debug->AddLine(nodePos,nodePos+charDiff,Color::RED);

   float dotProd = nodeForward.DotProduct(charDiff);

   if(dotProd < Cos(45.0f))
   {
       return false;
   }

   //debug->AddSphere(Sphere(nodePos,1.0f), Color::MAGENTA);

   Octree *octree = GetScene()->GetComponent<Octree>();
   Ray ray(nodePos + Vector3(0.0f, 1.6f, 0.0f) + (nodeForward * 0.25f), charDiff);
   PODVector<RayQueryResult> result;
   RayOctreeQuery query(result, ray, RAY_TRIANGLE, M_INFINITY, DRAWABLE_GEOMETRY);
   octree->RaycastSingle(query);
   //debug->AddLine(nodePos + Vector3(0.0f, 1.6f, 0.0f) + (nodeForward * 0.25f), nodePos + (charDiff * 100),Color::WHITE);
   if(result.Empty() || result[0].node_ != myChar)
   {
       return false;
   }

   return true;
}