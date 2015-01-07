#include "Pickup.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Person.h"
#include "ValueAnimation.h"


using namespace Urho3D;

const float Pickup::ROTATION_SPEED = 30.0f;
const float Pickup::HEIGHT_MIN = 0.2f;
const float Pickup::HEIGHT_MAX = 0.4f;

Pickup::Pickup(Context *context):
    LogicComponent(context),
    height_(0.0f)
{
}

void Pickup::RegisterObject(Context* context)
{
    context->RegisterFactory<Pickup>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
    ATTRIBUTE("Height", float, height_, 0.0f, AM_DEFAULT);
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

	if (!trigger_) {
		return;
	}

	//RigidBody *rigidBody = node_->GetComponent<RigidBody>();

	//PODVector<RigidBody *> colliders;
	//rigidBody->GetCollidingBodies(colliders);

	//Node *person = GetScene()->GetChild("Person", true);

	//if (!colliders.Empty() && colliders[0]->GetNode() == person) 
	//{
	//	LOGERROR("Collision");
	//	
	//}
	//else 
	//{
	//	
	//}


}

void Pickup::SetTriggerNode(Node *trigger)
{
	trigger_ = trigger;
}

Node *Pickup::GetTriggerNode() const
{
	return trigger_;
}
