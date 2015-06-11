#include "Urho3D/Urho3D.h"

#include "BossBertha.h"

#include "AudioManager.h"
#include "CameraController.h"
#include "Game.h"
#include "Person.h"

#include "Urho3D/Audio/Sound.h"
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
#include "Urho3D/Physics/CollisionShape.h"
#include "Urho3D/Physics/PhysicsEvents.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"

using namespace Urho3D;

const float BossBertha::CHARGE_SPEED = 4.0f;
const float BossBertha::CHARGE_INTERVAL = 5.0f;

BossBertha::BossBertha(Context *context) :
	InteractableComponent(context),
	chargeTimer_(0.0f),
	health_(1.0),
	healthBar_(NULL),
	healthText_(NULL)
{
}

void BossBertha::RegisterObject(Context* context)
{
	context->RegisterFactory<BossBertha>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void BossBertha::DelayedStart()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	frontMaterial_ = cache->GetResource<Material>("Materials/BerthaFront.xml");
	backMaterial_ = cache->GetResource<Material>("Materials/BerthaBack.xml");
	leftMaterial_ = cache->GetResource<Material>("Materials/BerthaLeft.xml");
	rightMaterial_ = cache->GetResource<Material>("Materials/BerthaRight.xml");

	model_ = node_->GetComponent<StaticModel>();

	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetMass(100.0f);
	rigidBody_->SetFriction(0.1f);
	rigidBody_->SetAngularFactor(Vector3::ZERO);

	CollisionShape *collisionShape = node_->GetComponent<CollisionShape>();
	collisionShape->SetCylinder(0.5f, 1.8f, Vector3(0.0f, 1.8f / 2.0f, 0.0f));

	SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(BossBertha, HandleNodeCollisionStart));
}


void BossBertha::Update(float timeStep)
{
	if (health_ <= 0.0f) {
		return;
	}

	UI *ui = GetSubsystem<UI>();

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	Vector3 position = node_->GetWorldPosition();

	Node *personNode = GetScene()->GetChild("Person", true);
	Vector3 personPosition = personNode->GetWorldPosition();

	IntVector2 room = IntVector2((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	IntVector2 personRoom = IntVector2((int)round(personPosition.x_ / 11.0f), (int)round(personPosition.z_ / 11.0f));

	bool inRoom = (room == personRoom);

	if (inRoom && !healthBar_) {
		healthBar_ = ui->GetRoot()->CreateChild<UIElement>();
		healthBar_->SetFixedSize(healthBar_->GetParent()->GetWidth(), 25);
		healthBar_->SetVerticalAlignment(VA_TOP);
		healthBar_->SetPosition(IntVector2(0, 25));
		healthBar_->SetVisible(true);

		Sprite *background = healthBar_->CreateChild<Sprite>();
		background->SetFixedSize(healthBar_->GetSize());
		background->SetColor(Color::RED);
		background->SetOpacity(0.75f);

		healthText_ = healthBar_->CreateChild<Text>();
		healthText_->SetFont("Fonts/Anonymous Pro.ttf");
		healthText_->SetColor(Color::WHITE);
		healthText_->SetText("Bertha Health: UNKNOWN");
		healthText_->SetAlignment(HA_CENTER, VA_CENTER);
		healthText_->SetTextAlignment(HA_CENTER);
	} else if (healthBar_ && !inRoom) {
		healthBar_->Remove();
		healthBar_ = NULL;
	}

	if (healthBar_) {
		healthText_->SetText("Bertha Health: " + String(round(health_ * 100.0f)) + "%");
	}

	if (!inRoom) {
		return;
	}

	Person *person = personNode->GetComponent<Person>();
	person->ShowHealth();

	Vector3 difference = personPosition - position;
	Quaternion rotation = Quaternion(node_->GetWorldDirection(), difference);

	bool isCharging = rigidBody_->GetLinearVelocity().LengthSquared() > 1.0f;

	if (isCharging) {
		return;
	}

	rigidBody_->SetLinearVelocity(Vector3::ZERO);

	if (Abs(rotation.YawAngle()) >= 90.0f) {
		model_->SetMaterial(frontMaterial_);
	} else {
		model_->SetMaterial(backMaterial_);
	}

	chargeTimer_ += timeStep;

	if (chargeTimer_ < CHARGE_INTERVAL) {
		return;
	}

	chargeTimer_ = 0.0f;

	// Play charge sound here?

	if (rotation.YawAngle() < 0.0f) {
		model_->SetMaterial(leftMaterial_);
	} else {
		model_->SetMaterial(rightMaterial_);
	}

	target_ = personPosition;

	rigidBody_->SetLinearVelocity(difference.Normalized() * CHARGE_SPEED);
}

void BossBertha::HandleNodeCollisionStart(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();
	Node *personNode = GetScene()->GetChild("Person", true);

	if (other != personNode) {
		return;
	}

	GetSubsystem<Game>()->EndLevel(true, false);
}

void BossBertha::TakeDamage(float damage)
{
	if (health_ <= 0.0f) {
		return;
	}

	health_ -= damage;

	if (health_ <= 0.0f) {
		health_ = 0.0f;

		ResourceCache *cache = GetSubsystem<ResourceCache>();

		Urho3D::PODVector<AudioQueueEntry> queue;
		queue.Push({ (Sound *)NULL, 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/29_Mav.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Voice/29_IHaveWork.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/28_YouInThere.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/27_Faith.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Voice/28_YouActuallyBeatHer.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/26_BREATHE.wav"), 0.0f });

		GetScene()->GetComponent<AudioManager>()->Play(queue);
	}
}
