#include "MrWright.h"

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

#include "InteractablePoster.h"

using namespace Urho3D;

MrWright::MrWright(Context *context):
	InteractableComponent(context)
{
}

void MrWright::RegisterObject(Context* context)
{
	context->RegisterFactory<MrWright>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void MrWright::DelayedStart()
{
	Node *roomNode = node_->GetParent();

	PODVector<Node *> posterNodes;
	roomNode->GetChildrenWithComponent<InteractablePoster>(posterNodes, true);

	PODVector<Material *> glyphs;
	for (PODVector<Node *>::ConstIterator i = posterNodes.Begin(); i != posterNodes.End(); ++i) {
		Node *posterNode = *i;
		StaticModel *poster = posterNode->GetComponent<StaticModel>();
		glyphs.Push(poster->GetMaterial());

		Vector3 position = posterNode->GetWorldPosition() - (posterNode->GetWorldDirection() * 1.5f);
		position.y_ = 0.0f;

		Node *interactionNode = roomNode->CreateChild();
		interactionNode->SetWorldPosition(position);
		interactionNode->SetWorldRotation(posterNode->GetWorldRotation());

		RigidBody *rigidBody = interactionNode->CreateComponent<RigidBody>();
		rigidBody->SetTrigger(true);

		CollisionShape *collisionShape = interactionNode->CreateComponent<CollisionShape>();
		collisionShape->SetBox(Vector3(1.0f, 3.0f, 0.5f), Vector3(0.0f, 1.5f, 0.0f));

		interactionNode->SetVar("poster", posterNode);

		SubscribeToEvent(interactionNode, E_NODECOLLISIONSTART, HANDLER(MrWright, HandleNodeCollision));
	}

	int glyphCount = 2;
	int sequenceCount = 3;

	for (int i = 0; i < sequenceCount; ++i) {
		PODVector<Material *> glyphPool = glyphs;
		PODVector<Material *> sequence;

		for (int j = 0; j < glyphCount; ++j) {
			int k = Random((int)glyphPool.Size());
			sequence.Push(glyphPool[k]);
			glyphPool.Erase(k);
		}

		sequences_.Push(sequence);

		glyphCount += 2;
	}

	UpdateDisplayGlyphs();
}

void MrWright::Update(float timeStep)
{
	swapTimer_ += timeStep;

	if (swapTimer_ < 10.0f) {
		return;
	}

	swapTimer_ = 0.0f;

	Node *roomNode = node_->GetParent();

	PODVector<Node *> posterNodes;
	roomNode->GetChildrenWithComponent<InteractablePoster>(posterNodes, true);

	int m = posterNodes.Size();
	while (m > 0) {
		int n = Random(0, m--);

		StaticModel *a = posterNodes[m]->GetComponent<StaticModel>();
		StaticModel *b = posterNodes[n]->GetComponent<StaticModel>();

		Material *t = a->GetMaterial();
		a->SetMaterial(b->GetMaterial());
		b->SetMaterial(t);
	}
}

void MrWright::HandleNodeCollision(StringHash eventType, VariantMap &eventData)
{
	Node *other = (Node *)eventData[NodeCollisionStart::P_OTHERNODE].GetPtr();

	Node *person = GetScene()->GetChild("Person", true);
	if (other != person) {
		return;
	}

	Node *node = (Node *)GetEventSender();
	Node *posterNode = (Node *)node->GetVar("poster").GetPtr();

	StaticModel *poster = posterNode->GetComponent<StaticModel>();
	Material *posterMaterial = poster->GetMaterial();

	LOGERRORF("Poster: %s", posterMaterial->GetName().CString());

	PODVector<Material *> &currentSequence = sequences_.Front();
	Material *&currentMaterial = currentSequence.Front(); // TIL that a reference to a pointer is valid.

	if (posterMaterial != currentMaterial) {
		LOGERROR("No Match!");
		return;
	}

	LOGERROR("Match!");

	int index = displayGlyphs_.Size() - currentSequence.Size();
	displayGlyphs_[index]->GetComponent<StaticModel>()->GetMaterial()->SetShaderParameter("MatDiffColor", Color::GREEN);

	index++;
	if (index < displayGlyphs_.Size()) {
		displayGlyphs_[index]->GetComponent<StaticModel>()->GetMaterial()->SetShaderParameter("MatDiffColor", Color::WHITE);
	}

	currentSequence.Erase(0);

	if (currentSequence.Empty()) {
		LOGERROR("Started Next Sequence!");
		sequences_.Erase(0);
		UpdateDisplayGlyphs();
	}
}

void MrWright::UpdateDisplayGlyphs()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();

	PODVector<Material *> &currentSequence = sequences_.Front();

	bool updateGlyphs = (displayGlyphs_.Size() != currentSequence.Size());

	while (displayGlyphs_.Size() != currentSequence.Size()) {
		if (displayGlyphs_.Size() < currentSequence.Size()) {
			Node *roomNode = node_->GetParent();

			Node *displayGlyph = roomNode->CreateChild();
			displayGlyph->SetWorldDirection(-node_->GetWorldDirection());
			displayGlyph->SetWorldScale(0.3f);

			StaticModel *glyph = displayGlyph->CreateComponent<StaticModel>();
			glyph->SetModel(cache->GetResource<Model>("Models/PersonPlane.mdl"));

			displayGlyphs_.Push(displayGlyph);
		} else {
			displayGlyphs_.Back()->Remove();
			displayGlyphs_.Pop();
		}
	}

	if (updateGlyphs) {
		for (int i = 0; i < displayGlyphs_.Size(); ++i) {
			Node *displayGlyph = displayGlyphs_[i];

			Vector3 position = node_->GetWorldPosition();
			position += Vector3((((displayGlyphs_.Size() - 1) * 0.6f) / -2.0f) + (i * 0.6f), 0.6f, 0.0f);
			position -= displayGlyph->GetWorldDirection() * 0.2f;

			displayGlyph->SetWorldPosition(position);

			StaticModel *glyph = displayGlyph->GetComponent<StaticModel>();
			glyph->SetMaterial(currentSequence[i]->Clone());

			if (i > 0) {
				glyph->GetMaterial()->SetShaderParameter("MatDiffColor", Color::GRAY);
			}
		}
	}
}
