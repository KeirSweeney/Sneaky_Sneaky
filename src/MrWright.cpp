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
#include "Terminal.h"


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
	

	//StaticModel *glyphModel = glyphs_[i] = glyphNode->CreateComponent<StaticModel>();
	//glyphModel->SetModel(cache->GetResource<Model>("PersonPlane.mdl"));
	//glyphModel->SetMaterial(cache->GetResource<Material>("Materials/Glyph" + String(i) + ".xml"));
	
	

}

void MrWright::DelayedStart()
{
 
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
	roomNode->GetChildrenWithComponent<Terminal>(terminalNodes, true);

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
				// at this point, were checing for the first terminal[5] and going through each poster to see if the diff is less than 0.5
				Vector3 thisTerminalPos = terminalNodes[x]->GetWorldPosition();
				Vector3 thisPosterPos = thisPosterNodes->GetWorldPosition();
				Vector3 diff = thisPosterPos - thisTerminalPos;
				float calc = abs(diff.LengthSquared());
				LOGERRORF("diff %f", calc);

				if (abs(diff.LengthSquared()) > 1.0f * 1.0f) {
					LOGERROR("Not Closest terminal");
					//break;// its not the closest 
				}
				else {
					StaticModel *c = thisPosterNodes->GetComponent<StaticModel>();
					LOGERROR("Found terminal");
					c->GetMaterial(); //this is the current material above the closest terminal?
					x--;
					if (x < 0) {
						goto LABEL;
					}
					//thisPosterNodes->Remove();
					//do what we need with it then break out
				}
				LOGERRORF("x: %d", x);
			}
		
	}
	
	LABEL:
	for (PODVector<Node *>::ConstIterator i = posterNodes.Begin(); i != posterNodes.End(); ++i) {
		Node *thisPosterNode = *i;

		
			
		}
	}



