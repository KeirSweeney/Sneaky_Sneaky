#include "Urho3D/Urho3D.h"

#include "MrWright.h"

#include "AudioManager.h"
#include "InteractablePoster.h"
#include "Person.h"

#include "Urho3D/Audio/Sound.h"
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
	if (sequences_.Empty()) {
		return;
	}

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

	if (sequences_.Empty()) {
		ResourceCache *cache = GetSubsystem<ResourceCache>();

		Urho3D::PODVector<AudioQueueEntry> queue;
		queue.Push({(Sound *)NULL, 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Black/16_Sigh.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Voice/16_KeepItSecure.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Black/15_Why.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Voice/15_CatFlap.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Black/14_Keys.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Voice/14_No.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Black/13_Petty.wav"), 0.0f});
		queue.Push({cache->GetResource<Sound>("Audio/VO/Voice/13_CheckPockets.wav"), 0.0f});

		GetScene()->GetComponent<AudioManager>()->Play(queue);
	}
}

void MrWright::UpdateDisplayGlyphs()
{
	if (sequences_.Empty()) {
		while (!displayGlyphs_.Empty()) {
			displayGlyphs_.Back()->Remove();
			displayGlyphs_.Pop();
		}

		return;
	}

	ResourceCache *cache = GetSubsystem<ResourceCache>();

	PODVector<Material *> &currentSequence = sequences_.Front();

	bool updateGlyphs = (displayGlyphs_.Size() != currentSequence.Size());

	while (displayGlyphs_.Size() != currentSequence.Size()) {
		if (displayGlyphs_.Size() < currentSequence.Size()) {
			Node *roomNode = node_->GetParent();

			Node *displayGlyph = roomNode->CreateChild();
			displayGlyph->SetWorldDirection(Vector3::DOWN);
			displayGlyph->SetWorldScale(0.3f);

			StaticModel *glyph = displayGlyph->CreateComponent<StaticModel>();
			glyph->SetModel(cache->GetResource<Model>("Models/PersonPlane.mdl"));
			glyph->SetViewMask(0x02);

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
			position += Vector3((((displayGlyphs_.Size() - 1) * 0.6f) / -2.0f) + (i * 0.6f), 0.0f, -1.0f);
			displayGlyph->SetWorldPosition(position);

			StaticModel *glyph = displayGlyph->GetComponent<StaticModel>();
			glyph->SetMaterial(currentSequence[i]->Clone());

			if (i > 0) {
				glyph->GetMaterial()->SetShaderParameter("MatDiffColor", Color::GRAY);
			}
		}
	}
}
