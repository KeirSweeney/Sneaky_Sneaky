#include "Urho3D/Urho3D.h"

#include "AudioManager.h"
#include "Game.h"
#include "InteractablePoster.h"
#include "Person.h"
#include "Terminal.h"
#include "Wife.h"

#include "Urho3D/Audio/Sound.h"
#include "Urho3D/Audio/SoundSource.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Core/Profiler.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/Model.h"
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
#include "Urho3D/UI/UIElement.h"

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
