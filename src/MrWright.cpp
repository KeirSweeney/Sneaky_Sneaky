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
#include "MrWrightTerminal.h"

using namespace Urho3D;

MrWright::MrWright(Context *context):
	InteractablePoster(context)
{
}

void MrWright::RegisterObject(Context* context)
{
	context->RegisterFactory<MrWright>("Logic");

	COPY_BASE_ATTRIBUTES(InteractablePoster);
}

void MrWright::Start()
{
	LoadMaterials();
}

void MrWright::LoadMaterials()
{
	int glyphCount = 3;
	int sequenceCount = 3;

	Node *roomNode = node_->GetParent();

	PODVector<Node *> posterNodes;
	roomNode->GetChildrenWithComponent<InteractablePoster>(posterNodes, true);

	PODVector<Material *> glyphs;
	for (PODVector<Node *>::ConstIterator i = posterNodes.Begin(); i != posterNodes.End(); ++i) {
		StaticModel *poster = (*i)->GetComponent<StaticModel>();
		glyphs.Push(poster->GetMaterial());
	}

	for (int i = 0; i < sequenceCount; ++i) {
		PODVector<Material *> glyphPool = glyphs;
		PODVector<Material *> sequence;

		for (int j = 0; j < glyphCount; ++j) {
			int k = Random((int)glyphPool.Size());
			sequence.Push(glyphPool[k]);
			glyphPool.Erase(k);
		}

		sequences_.Push(sequence);
		glyphCount++;
	}
}

void MrWright::Update(float timeStep)
{
	swapTimer_ += timeStep;


	if (swapTimer_ < 5.0f) {
		return;
	}

	swapTimer_ = 0.0f;

	Node *roomNode = node_->GetParent();

	PODVector<Node *> posterNodes;
	PODVector<Node *> terminalNodes;

	roomNode->GetChildrenWithComponent<InteractablePoster>(posterNodes, true);
	roomNode->GetChildrenWithComponent<MrWrightTerminal>(terminalNodes, true);

	if (posterNodes.Empty()) {
		LOGERROR("Poster nodes are empty");
	}

	if (terminalNodes.Empty()) {
		LOGERROR("Terminal nodes are empty");
	}

	ResourceCache *cache = GetSubsystem<ResourceCache>();

	int m = posterNodes.Size();
	while (m > 0) {
		int n = Random(0, m--);

		StaticModel *a = posterNodes[m]->GetComponent<StaticModel>();
		StaticModel *b = posterNodes[n]->GetComponent<StaticModel>();

		Material *t = a->GetMaterial();
		a->SetMaterial(b->GetMaterial());
		b->SetMaterial(t);
	}

	int x = terminalNodes.Size();
	LOGERRORF("terminal nodes size: %d", x);
	x -= 1;
	int n = posterNodes.Size();
	LOGERRORF("poster nodes size %d", n);

	while (x >= 0) {

			for (PODVector<Node *>::ConstIterator i = posterNodes.Begin(); i != posterNodes.End(); ++i) {

				Node *thisPosterNodes = *i;
				Vector3 thisTerminalPos = terminalNodes[x]->GetWorldPosition();
				Vector3 thisPosterPos = thisPosterNodes->GetWorldPosition();
				Vector3 diff = thisPosterPos - thisTerminalPos;
				float calc = Abs(diff.LengthSquared());

				if (calc > 1.0f * 1.0f) {
					LOGERROR("Not Closest terminal");
				}
				else {
					StaticModel *c = thisPosterNodes->GetComponent<StaticModel>();
					LOGERROR("Found terminal");
					String str = c->GetMaterial()->GetName();

					MrWrightTerminal *thisTerm = terminalNodes[x]->GetComponent<MrWrightTerminal>();
					thisTerm->SetContent(str);
					//this is the current material above the closest terminal?
					LOGERRORF("MAterial NAme %s", str.CString()); // store a vector of 3 materials in an order, then if the terminals material matches the first one in the vector, remove that one from the vector
					x--;
					if (x < 0) {
						goto LABEL;
					}
					//do what we need with it then break out
				}
				LOGERRORF("x: %d", x);
			}

	}

LABEL:;

	}
