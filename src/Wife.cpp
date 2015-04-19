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

#include "InteractablePoster.h"

using namespace Urho3D;

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

	PODVector<Node *> terminalNodes;
	roomNode->GetChildrenWithComponent<Terminal>(terminalNodes, true);

	//PODVector<Node *> terminals;
	for (int i = 0; i < terminalNodes.Size(); ++i) {
		Node *terminalNode = terminalNodes[i];
		terminals_.Push(terminalNode);
		//StaticModel *terminal = terminalNode->GetComponent<StaticModel>();
		//terminals.Push(terminalNode);

		Vector3 position = terminalNode->GetWorldPosition() - (terminalNode->GetWorldDirection() * 1.5f);
		position.y_ = 0.0f;

		Node *interactionNode = roomNode->CreateChild();
		interactionNode->SetWorldPosition(position);
		interactionNode->SetWorldRotation(terminalNode->GetWorldRotation());

		RigidBody *rigidBody = interactionNode->CreateComponent<RigidBody>();
		rigidBody->SetTrigger(true);

		CollisionShape *collisionShape = interactionNode->CreateComponent<CollisionShape>();
		collisionShape->SetBox(Vector3(1.0f, 3.0f, 0.5f), Vector3(0.0f, 1.5f, 0.0f));

		interactionNode->SetVar("index", i);

		SubscribeToEvent(interactionNode, E_NODECOLLISIONSTART, HANDLER(Wife, HandleNodeCollision));

		terminalNode->RemoveComponent<Terminal>();
	}

	int terminalCount = 5;

	PODVector<Node *> terminalPool = terminals_;
	PODVector<int> sequence;

	for (int j = 0; j < terminalCount; ++j) {
		int k = Random((int)terminalPool.Size());
		sequence.Push(terminalPool[k]->GetVar("index").GetInt());
		terminalPool.Erase(k);
	}

	sequences_.Push(sequence);

}

void Wife::Update(float timeStep)
{
	time_ += timeStep;

	Node *roomNode = node_->GetParent();

}

void Wife::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();

	Node *person = GetScene()->GetChild("Person", true);
	if (other != person) {
		return;
	}

	Node *node = (Node *)GetEventSender();
	//Node *terminalNode = (Node *)node->GetVar("terminal").GetPtr();

	int terminalID = (int)node->GetVar("index").GetInt();

	int currentSequence = sequences_.Front();
	//int nextInt = currentSequence->GetVar("terminal").GetInt();

	//PODVector<Node *> currentSequence = sequences_.Front();
	//Material *&currentMaterial = currentSequence.Front(); // TIL that a reference to a pointer is valid.

	if (currentSequence != terminalID) {
		LOGERROR("No Match!");
		return;
	}

	LOGERROR("Match!");

	terminals_[terminalID]->GetComponent<StaticModel>()->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.5f, 1.0f, 0.5f));

	/*index++;
	if (index < terminals_.Size()) {
		terminals_[terminalID]->GetComponent<StaticModel>()->GetMaterial()->SetShaderParameter("MatDiffColor", Color::WHITE);
	}*/

	sequences_.Erase(0);

	if (sequences_.Empty()) {
		LOGERROR("Completed");
	}
}
