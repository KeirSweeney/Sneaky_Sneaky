#include "Laser.h"

#include "CameraController.h"
#include "Game.h"
#include "Guard.h"

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

using namespace Urho3D;

Laser::Laser(Context *context):
	InteractableComponent(context),
	lightPulse_(false),
	lightTime_(0.0f), laserTime_(0.0f),
	laserInterval_(0.0f), laserDelay_(0.0f)
{
}

void Laser::RegisterObject(Context* context)
{
	context->RegisterFactory<Laser>("Logic");
	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void Laser::LoadFromXML(const XMLElement &xml)
{
	laserInterval_ = xml.GetFloat("interval");
	laserDelay_ = xml.GetFloat("delay");
}

void Laser::DelayedStart()
{
	source_ = node_->CreateComponent<SoundSource>();

	model_ = node_->GetComponent<StaticModel>();
	model_->SetEnabled(false);

	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetTrigger(true);

	SubscribeToEvent(node_, E_NODECOLLISION, HANDLER(Laser, HandleNodeCollision));
}

void Laser::Update(float timeStep)
{
	if (laserDelay_ > 0.0f) {
		laserDelay_ -= timeStep;
		return;
	}

	if(laserInterval_ > 0.0f) {
		laserTime_ += timeStep;

		if(laserTime_ > laserInterval_) {
			model_->SetEnabled(!model_->IsEnabled());
			laserTime_ = 0.0f;
		}
	}

	if(!lightPulse_) {
		source_->Stop();
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
		Light *light = (*i)->GetComponent<Light>();

		if (light->GetLightType() != LIGHT_POINT) {
			continue;
		}

		if(lightPulse_) {
			light->SetColor(Color::RED);
			light->SetBrightness(Abs(Sin(lightTime_ * 360.0f)) * 0.4f);
		} else {
			light->SetColor(Color::WHITE);
			light->SetBrightness(0.2f);
		}
	}
}

void Laser::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	StaticModel *laserModel = node_->GetComponent<StaticModel>();

	//work on de-activiating lasers with terminals. Terminal class, create new components, instead of message state, do laser state, if it is a laser, then find all the lasers in the room node and disable the lasers
	if(!laserModel->IsEnabled()) {
		return;
	}

	Node *personNode = GetScene()->GetChild("Person", true);
	Node *other = (Node *)eventData[NodeCollision::P_OTHERNODE].GetPtr();

	if(other != personNode) {
		return;
	}

	lightPulse_ = true;
	Alarm();


	NavigationMesh *navMesh = GetScene()->GetComponent<NavigationMesh>();

	Vector3 pointPath = personNode->GetWorldPosition();

	Node *roomNode = node_->GetParent()->GetParent();

	PODVector<Node *> nodes;
	roomNode->GetChildrenWithComponent<Guard>(nodes,true);

	for (PODVector<Node *>::ConstIterator i =nodes.Begin(); i != nodes.End(); ++i) {
		Node *guardNode = *i;


		Urho3D::PODVector<Urho3D::Vector3> path;
		navMesh->FindPath(path, guardNode->GetWorldPosition(), pointPath);

		Guard *guard = guardNode->GetComponent<Guard>();
		guard->SetPath(path);
	}
}
void Laser::Alarm()
{
	if (!source_->IsPlaying())
	{
		ResourceCache *cache = GetSubsystem<ResourceCache>();
		Sound *alarm = cache->GetResource<Sound>("Audio/Alarm.ogg");
		source_->SetSoundType(SOUND_EFFECT);
		source_->SetGain(0.5f);
		source_->Play(alarm);
	}
}

