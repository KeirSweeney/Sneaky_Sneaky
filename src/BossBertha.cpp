#include "BossBertha.h"

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
#include "Renderer.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "StaticModel.h"
#include "UI.h"
#include "PhysicsEvents.h"
#include "CollisionShape.h"
#include "Sprite.h"
#include "Text.h"
#include "AudioManager.h"
#include "Sound.h"

using namespace Urho3D;

const float BossBertha::CHARGE_SPEED = 4.0f;
const float BossBertha::CHARGE_INTERVAL = 5.0f;
const int BossBertha::PADDING = 20;

BossBertha::BossBertha(Context *context) :
	InteractableComponent(context),
	health_(200),
	chargeTimer_(0.0f)
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

	UI *ui = GetSubsystem<UI>();
	//Node *personNode = GetScene()->GetChild("Person", true);
	//Person *person = personNode->GetComponent<Person>();

	int x = PADDING;
	int y = PADDING;
	UIElement *berthaHealth = ui->GetRoot()->CreateChild<UIElement>();
	berthaHealth->SetFixedSize(berthaHealth->GetParent()->GetSize() - IntVector2(PADDING * 40, PADDING * 35));
	berthaHealth->SetAlignment(HA_RIGHT, VA_TOP);
	berthaHealth->SetVisible(true);

	Sprite *background = berthaHealth->CreateChild<Sprite>();
	background->SetFixedSize(berthaHealth->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.7f);

	Text *label = berthaHealth->CreateChild<Text>();
	label->SetFixedSize(berthaHealth->GetSize() - IntVector2(PADDING, PADDING));
	label->SetFont("Fonts/Anonymous Pro.ttf");
	label->SetColor(Color::WHITE);
	label->SetText("Bertha Health: " + (String)GetHealth());
	label->SetAlignment(HA_CENTER, VA_CENTER);
	label->SetTextAlignment(HA_CENTER);
	label->SetWordwrap(true);

	berthaHealth_ = berthaHealth->CreateChild<UIElement>();
	berthaHealth_->SetFixedSize(berthaHealth->GetSize());

	model_ = node_->GetComponent<StaticModel>();

	rigidBody_ = node_->GetComponent<RigidBody>();
	rigidBody_->SetMass(100.0f);
	rigidBody_->SetFriction(0.0f);
	rigidBody_->SetAngularFactor(Vector3::ZERO);

	CollisionShape *collisionShape = node_->GetComponent<CollisionShape>();
	collisionShape->SetCylinder(0.5f, 1.8f, Vector3(0.0f, 1.8f / 2.0f, 0.0f));

	SubscribeToEvent(node_, E_NODECOLLISIONSTART, HANDLER(BossBertha, HandleNodeCollisionStart));
}


void BossBertha::Update(float timeStep)
{
	if (isDirty()) {
		berthaHealth_->RemoveAllChildren();
		UI *ui = GetSubsystem<UI>();
		//set dirty in somewher which will enable the setvisible to true

		int x = PADDING;
		int y = PADDING;
		UIElement *bossHealth = ui->GetRoot()->CreateChild<UIElement>();
		bossHealth->SetFixedSize(bossHealth->GetParent()->GetSize() - IntVector2(PADDING * 40, PADDING * 35));
		bossHealth->SetAlignment(HA_RIGHT, VA_TOP);
		bossHealth->SetVisible(true);

		Sprite *background = bossHealth->CreateChild<Sprite>();
		background->SetFixedSize(bossHealth->GetSize());
		background->SetColor(Color::BLACK);
		background->SetOpacity(0.7f);

		Text *label = bossHealth->CreateChild<Text>();
		label->SetFixedSize(bossHealth->GetSize() - IntVector2(PADDING, PADDING));
		label->SetFont("Fonts/Anonymous Pro.ttf");
		label->SetColor(Color::WHITE);
		label->SetText("Bertha Health: " + (String)GetHealth());
		label->SetAlignment(HA_CENTER, VA_CENTER);
		label->SetTextAlignment(HA_CENTER);
		label->SetWordwrap(true);

		berthaHealth_ = bossHealth->CreateChild<UIElement>();
		berthaHealth_->SetFixedSize(bossHealth->GetSize());
	}



	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	CameraController *cameraController = camera->GetNode()->GetParent()->GetComponent<CameraController>();
	node_->SetWorldRotation(Quaternion(cameraController->GetYawAngle(), Vector3::UP));

	Vector3 position = node_->GetWorldPosition();

	Node *personNode = GetScene()->GetChild("Person", true);
	Vector3 personPosition = personNode->GetWorldPosition();

	IntVector2 room = IntVector2((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	IntVector2 personRoom = IntVector2((int)round(personPosition.x_ / 11.0f), (int)round(personPosition.z_ / 11.0f));

	if (room != personRoom) {
		return;
	}

	Vector3 difference = personPosition - position;
	Quaternion rotation = Quaternion(node_->GetWorldDirection(), difference);

	bool isCharging = rigidBody_->GetLinearVelocity().LengthSquared() > 0.1f;

	if (isCharging) {
		// Have her follow the player just a tiny bit.
		target_ = target_.Lerp(personPosition, 1.0f * timeStep);

		Vector3 offset = target_ - position;

		rigidBody_->SetLinearVelocity(offset.Normalized() * CHARGE_SPEED);

		if (offset.LengthSquared() < (CHARGE_SPEED * CHARGE_SPEED * timeStep * timeStep)) {
			rigidBody_->SetLinearVelocity(Vector3::ZERO);

			// Some kind of skid sound?
		}

		return;
	}

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

	if (health_ <= 0) {
		ResourceCache *cache = GetSubsystem<ResourceCache>();

		Urho3D::PODVector<AudioQueueEntry> queue;
		queue.Push({ (Sound *)NULL, 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/26_BREATHE.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Voice/28_YouActuallyBeatHer.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/27_Faith.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/28_YouInThere.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Voice/29_IHaveWork.wav"), 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/29_Mav.wav"), 0.0f });

		GetScene()->GetComponent<AudioManager>()->Play(queue);
	}
		//GetSubsystem<Game>()->EndLevel(true, false);
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

void BossBertha::TakeDamage()
{
	health_ -= 1;
	takingDamage_ = true;

}

int BossBertha::GetHealth()
{
	return health_;
}

bool BossBertha::isDirty()
{
	if (!takingDamage_) {
		return false;
	}

	return true;
}
