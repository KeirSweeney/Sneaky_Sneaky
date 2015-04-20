#include "Wife.h"

#include "Context.h"
#include "Node.h"
#include "Scene.h"
#include "NavigationMesh.h"
#include "Log.h"
#include "DebugRenderer.h"
#include "Profiler.h"
#include "RigidBody.h"
#include "Person.h"
#include "UI.h"
#include "Camera.h"
#include "Sprite.h"
#include "Graphics.h"
#include "UIElement.h"
#include "Text.h"
#include "Renderer.h"
#include "StaticModel.h"
#include "ResourceCache.h"
#include "Model.h"
#include "Material.h"
#include "Input.h"
#include "CollisionShape.h"
#include "PhysicsEvents.h"
#include "Terminal.h"
#include "Sound.h"
#include "AudioManager.h"
#include "Game.h"

#include "InteractablePoster.h"

using namespace Urho3D;

const int Wife::PADDING = 20;

Wife::Wife(Context *context):
	InteractableComponent(context)
{
}

void Wife::RegisterObject(Context* context)
{
	context->RegisterFactory<Wife>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void Wife::DelayedStart()
{
	UI *ui = GetSubsystem<UI>();
	Node *personNode = GetScene()->GetChild("Person", true);
	Person *person = personNode->GetComponent<Person>();

	int x = PADDING;
	int y = PADDING;
	UIElement *playerHealth = ui->GetRoot()->CreateChild<UIElement>();
	playerHealth->SetFixedSize(playerHealth->GetParent()->GetSize() - IntVector2(PADDING * 40, PADDING * 35));
	playerHealth->SetAlignment(HA_LEFT, VA_TOP);
	playerHealth->SetVisible(true);

	Sprite *background = playerHealth->CreateChild<Sprite>();
	background->SetFixedSize(playerHealth->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.7f);

	Text *label = playerHealth->CreateChild<Text>();
	label->SetFixedSize(playerHealth->GetSize() - IntVector2(PADDING, PADDING));
	label->SetFont("Fonts/Anonymous Pro.ttf");
	label->SetColor(Color::WHITE);
	label->SetText("Player Health: " + (String)person->GetHealth());
	label->SetAlignment(HA_CENTER, VA_CENTER);
	label->SetTextAlignment(HA_CENTER);
	label->SetWordwrap(true);

	playerHealth_ = playerHealth->CreateChild<UIElement>();
	playerHealth_->SetFixedSize(playerHealth->GetSize());

	LOGERROR("WIFE CLASS!");
	Node *roomNode = node_->GetParent();

	PODVector<Node *> terminals;
	roomNode->GetChildrenWithComponent<Terminal>(terminals, true);

	for (PODVector<Node *>::ConstIterator i = terminals.Begin(); i != terminals.End(); ++i) {
		Node *terminalNode = *i;

		terminalNode->RemoveComponent<Terminal>();

		Vector3 position = terminalNode->GetWorldPosition() - (terminalNode->GetWorldDirection() * 0.75f);
		position.y_ = 0.0f;

		Node *interactionNode = roomNode->CreateChild();
		interactionNode->SetWorldPosition(position);
		interactionNode->SetWorldRotation(terminalNode->GetWorldRotation());
		interactionNode->SetVar("terminal", terminalNode);

		RigidBody *rigidBody = interactionNode->CreateComponent<RigidBody>();
		rigidBody->SetTrigger(true);

		CollisionShape *collisionShape = interactionNode->CreateComponent<CollisionShape>();
		collisionShape->SetBox(Vector3(1.0f, 3.0f, 0.5f), Vector3(0.0f, 1.5f, 0.0f));

		SubscribeToEvent(interactionNode, E_NODECOLLISIONSTART, HANDLER(Wife, HandleNodeCollision));
	}

	sequence_ = terminals;

	int m = sequence_.Size();
	while (m > 0) {
		int n = Random(0, m--);
		Swap(sequence_[m], sequence_[n]);
	}
}

void Wife::Update(float timeStep)
{
	time_ += timeStep;

	Node *roomNode = node_->GetParent();
	Node *personNode = GetScene()->GetChild("Person", true);
	Person *person = personNode->GetComponent<Person>();

	if (person->isDirty()) {
		playerHealth_->RemoveAllChildren();
		UI *ui = GetSubsystem<UI>();
		//set dirty in somewher which will enable the setvisible to true

		int x = PADDING;
		int y = PADDING;
		UIElement *playerHealth = ui->GetRoot()->CreateChild<UIElement>();
		playerHealth->SetFixedSize(playerHealth->GetParent()->GetSize() - IntVector2(PADDING * 40, PADDING * 35));
		playerHealth->SetAlignment(HA_LEFT, VA_TOP);
		playerHealth->SetVisible(true);

		Sprite *background = playerHealth->CreateChild<Sprite>();
		background->SetFixedSize(playerHealth->GetSize());
		background->SetColor(Color::BLACK);
		background->SetOpacity(0.7f);

		Text *label = playerHealth->CreateChild<Text>();
		label->SetFixedSize(playerHealth->GetSize() - IntVector2(PADDING, PADDING));
		label->SetFont("Fonts/Anonymous Pro.ttf");
		label->SetColor(Color::WHITE);
		label->SetText("Player Health: " + (String)person->GetHealth());
		label->SetAlignment(HA_CENTER, VA_CENTER);
		label->SetTextAlignment(HA_CENTER);
		label->SetWordwrap(true);

		playerHealth_ = playerHealth->CreateChild<UIElement>();
		playerHealth_->SetFixedSize(playerHealth->GetSize());
	}

	if (person->GetHealth() <= 0) {
		GetSubsystem<Game>()->EndLevel(true);
	}
	

}

void Wife::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	if (sequence_.Empty()) {
		return;
	}

	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();

	Node *person = GetScene()->GetChild("Person", true);
	if (other != person) {
		return;
	}

	Node *node = (Node *)GetEventSender();
	Node *terminal = (Node *)node->GetVar("terminal").GetPtr();

	LOGERRORF("Node: %d, Terminal: %d, Next: %d", node->GetID(), terminal->GetID(), sequence_.Front()->GetID());

	if (terminal != sequence_.Front()) {
		LOGERROR("No Match!");
		return;
	}

	LOGERROR("Match!");

	StaticModel *terminalModel = terminal->GetComponent<StaticModel>();
	terminalModel->SetMaterial(terminalModel->GetMaterial()->Clone());
	terminalModel->GetMaterial()->SetShaderParameter("MatDiffColor", Color(5.0f, 20.0f, 5.0f));

	sequence_.Erase(0);

	if (sequence_.Empty()) {
		ResourceCache *cache = GetSubsystem<ResourceCache>();

		Urho3D::PODVector<AudioQueueEntry> queue;
		queue.Push({ (Sound *)NULL, 0.0f });
		queue.Push({ cache->GetResource<Sound>("Audio/VO/Black/32_IClearedMyName.wav"), 0.0f });

		GetScene()->GetComponent<AudioManager>()->Play(queue);
	}
}
