#include "Urho3D/Urho3D.h"

#include "BossLaser.h"

#include "BossBertha.h"
#include "CameraController.h"
#include "Game.h"
#include "Person.h"

#include "Urho3D/Audio/Sound.h"
#include "Urho3D/Audio/SoundSource.h"
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
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/UI.h"

using namespace Urho3D;


BossLaser::BossLaser(Context *context):
	InteractableComponent(context),
	laserTime_(0.0f), laserInterval_(0.0f), laserDelay_(0.0f)
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
	Node *other = (Node *)eventData[NodeCollision::P_OTHERNODE].GetPtr();

	if (other == personNode) {
		Person *person = personNode->GetComponent<Person>();
		person->TakeDamage(0.005f);
	} else if (other->HasComponent<BossBertha>()) {
		BossBertha *bertha = other->GetComponent<BossBertha>();
		bertha->TakeDamage(0.005f);
		LOGERROR("Bertha Collision");
	}
}
