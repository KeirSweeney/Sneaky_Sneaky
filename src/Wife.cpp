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
#include "SoundSource.h"

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
	Vector3 position = node_->GetWorldPosition();

	Node *personNode = GetScene()->GetChild("Person", true);
	Vector3 personPosition = personNode->GetWorldPosition();

	IntVector2 room = IntVector2((int)round(position.x_ / 11.0f), (int)round(position.z_ / 11.0f));
	IntVector2 personRoom = IntVector2((int)round(personPosition.x_ / 11.0f), (int)round(personPosition.z_ / 11.0f));

	if (room != personRoom) {
		return;
	} else {
		Person *person = personNode->GetComponent<Person>();
		person->ShowHealth();
	}

	if (sequence_.Empty()) {
		// The ending audio is played on the credits, which we want to happen immediately.
		GetSubsystem<Game>()->EndLevel(false, true);

		return;
	}

	time_ += timeStep;
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

	if (terminal != sequence_.Front()) {
		LOGERROR("No Match!");

		SoundSource *source = terminal->CreateComponent<SoundSource>();
		source->SetAutoRemove(true);
		source->Play(GetSubsystem<ResourceCache>()->GetResource<Sound>("Audio/Windows Hardware Fail.wav"));

		return;
	}

	LOGERROR("Match!");

	SoundSource *source = terminal->CreateComponent<SoundSource>();
	source->SetAutoRemove(true);
	source->Play(GetSubsystem<ResourceCache>()->GetResource<Sound>("Audio/Windows Hardware Insert.wav"));

	StaticModel *terminalModel = terminal->GetComponent<StaticModel>();
	terminalModel->SetMaterial(terminalModel->GetMaterial()->Clone());
	terminalModel->GetMaterial()->SetShaderParameter("MatDiffColor", Color(5.0f, 20.0f, 5.0f));

	sequence_.Erase(0);
}
