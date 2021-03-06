#include "Urho3D/Urho3D.h"

#include "Pickup.h"

#include "Inventory.h"
#include "Person.h"

#include "Urho3D/Audio/Audio.h"
#include "Urho3D/Audio/Sound.h"
#include "Urho3D/Audio/SoundSource3D.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Scene/ValueAnimation.h"

using namespace Urho3D;

const float Pickup::ROTATION_SPEED = 30.0f;
const float Pickup::HEIGHT_MIN = 0.2f;
const float Pickup::HEIGHT_MAX = 0.4f;

Pickup::Pickup(Context *context):
	InteractableComponent(context),
	height_(0.0f)
{
}

void Pickup::RegisterObject(Context* context)
{
	context->RegisterFactory<Pickup>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
	ATTRIBUTE("Height", float, height_, 0.0f, AM_DEFAULT);
}

void Pickup::LoadFromXML(const XMLElement &xml)
{
	type_ = xml.HasAttribute("type") ? xml.GetAttribute("type") : xml.GetAttribute("model");
	name_ = xml.HasAttribute("name") ? xml.GetAttribute("name") : type_;
}

void Pickup::DelayedStart()
{
	node_->GetComponent<RigidBody>()->SetTrigger(true);

	SharedPtr<ValueAnimation> animation(new ValueAnimation(context_));
	animation->SetInterpolationMethod(IM_SPLINE);
	animation->SetSplineTension(0.0f);
	animation->SetKeyFrame(0.0f, HEIGHT_MIN);
	animation->SetKeyFrame(1.0f, HEIGHT_MAX);
	animation->SetKeyFrame(2.0f, HEIGHT_MIN);
	SetAttributeAnimation("Height", animation);
}

void Pickup::Update(float timeStep)
{
	node_->Rotate(Quaternion(ROTATION_SPEED * timeStep, Vector3::UP));

	Vector3 position = node_->GetPosition();
	position.y_ = height_;
	node_->SetPosition(position);

	RigidBody *rigidBody = node_->GetComponent<RigidBody>();

	PODVector<RigidBody *> colliders;
	rigidBody->GetCollidingBodies(colliders);

	Node *person = GetScene()->GetChild("Person", true);

	if (!colliders.Empty() && colliders[0]->GetNode() == person) {
		Inventory *inventory = person->GetComponent<Inventory>();
		inventory->AddItem(this);

		ResourceCache *cache = GetSubsystem<ResourceCache>();

		Node *sound = GetScene()->CreateChild("SoundEffect");
		sound->SetWorldPosition(node_->GetWorldPosition());

		SoundSource *source = sound->CreateComponent<SoundSource>();
		source->SetAutoRemove(true);
		source->SetSoundType(SOUND_EFFECT);
		source->Play(cache->GetResource<Sound>("Audio/Pickup.wav"));

		node_->SetEnabled(false);
	}
}

String Pickup::GetPickupType()
{
	return type_;
}

String Pickup::GetDisplayName()
{
	return name_;
}


