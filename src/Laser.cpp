#include "Laser.h"

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
#include "PhysicsEvents.h"
#include "Guard.h"

using namespace Urho3D;

Laser::Laser(Context *context):
    LogicComponent(context),
    lightPulse_(false),
    lightTime_(0.0f)
{
}

void Laser::RegisterObject(Context* context)
{
	context->RegisterFactory<Laser>("Logic");
	COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Laser::Update(float timeStep)
{
    if(!lightPulse_) {
        return;
    }
    lightTime_ += timeStep * 0.5f;
    if(lightTime_ > 5.0f) {
        lightPulse_= false;
        lightTime_ = 0;
    }
    Node *roomNode = node_->GetParent()->GetParent();
    PODVector<Node *> nodes;

    roomNode->GetChildrenWithComponent<Light>(nodes,true);

    for (PODVector<Node *>::ConstIterator i =nodes.Begin(); i != nodes.End(); ++i) {
        Node *lightNode = *i;

        Light *light = lightNode->GetComponent<Light>();

        if(lightPulse_) {
            light->SetColor(Color::RED);
            light->SetBrightness(Abs(Sin(lightTime_*360.0f)) * 0.4f);
        }
        else {
            light->SetColor(Color::WHITE);
            light->SetBrightness(0.2f);
        }
    }


}

void Laser::DelayedStart()
{
	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetTrigger(true);
	SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(Laser, HandleNodeCollisionStart));
}

void Laser::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	Node *personNode = GetScene()->GetChild("Person", true);
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();

	if(other != personNode) {
		return;
	}

    lightPulse_ = true;

	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

	Vector3 pointPath = personNode->GetWorldPosition();

    Node *roomNode = node_->GetParent()->GetParent();
    PODVector<Node *> nodes;

	roomNode->GetChildrenWithComponent<Guard>(nodes,true);

	for (PODVector<Node *>::ConstIterator i =nodes.Begin(); i != nodes.End(); ++i) {
		Node *guardNode = *i;

		Guard *guard = guardNode->GetComponent<Guard>();

		navMesh->FindPath(path_,guardNode->GetWorldPosition(),pointPath);
		guard->SetPath(path_);
	}
}
