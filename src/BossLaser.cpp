#include "BossLaser.h"

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
#include "PhysicsEvents.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sound.h"
#include "SoundSource.h"
#include "StaticModel.h"
#include "UI.h"
#include "Person.h"


using namespace Urho3D;


BossLaser::BossLaser(Context *context):
	InteractableComponent(context),
	lightPulse_(false),
	lightTime_(0.0f), laserTime_(0.0f),
	laserInterval_(0.0f), laserDelay_(0.0f)
{}

void BossLaser::RegisterObject(Context *context)
{
	context->RegisterFactory<BossLaser>("Logic");
	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void BossLaser::LoadFromXML(const XMLElement &xml)
{
	laserInterval_ = xml.GetFloat("interval");
	laserDelay_ = xml.GetFloat("delay");
}

void BossLaser::DelayedStart()
{
	source_ = node_->CreateComponent<SoundSource>();

	model_ = node_->GetComponent<StaticModel>();
	model_->SetEnabled(false);

	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetTrigger(true);

	SubscribeToEvent(node_, E_NODECOLLISION, HANDLER(BossLaser, HandleNodeCollision));
}


void BossLaser::Update(float timeStep)
{

	if (laserDelay_ > 0.0f) {
		laserDelay_ -= timeStep;
		return;
	}

	if (laserInterval_ > 0.0f) {
		laserTime_ += timeStep;

		if (laserTime_ > laserInterval_) {
			model_->SetEnabled(!model_->IsEnabled());
			laserTime_ = 0.0f;
		}
	}

}


void BossLaser::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	StaticModel *laserModel = node_->GetComponent<StaticModel>();

	if (!laserModel->IsEnabled()) {
		return;
	}

	Node *personNode = GetScene()->GetChild("Person", true);
	Person *person = personNode->GetComponent<Person>();
	Node *other = (Node *)eventData[NodeCollision::P_OTHERNODE].GetPtr();

	if (other != personNode) {
		return;
	}

	lightPulse_ = true;

	//Deduct player hp, do once
	person->TakeDamage(0.005f);
}
