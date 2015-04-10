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
	PODVector<Node *> nodes;
	roomNode->GetChildrenWithComponent<InteractablePoster>(nodes, true);

	if (nodes.Empty()) {
		LOGERROR("ITS EMPTY");
	}
	
	
	ResourceCache *cache = GetSubsystem<ResourceCache>();


	int m = nodes.Size();
	while (m > 0) {
		int n = Random(0, m--);

		StaticModel *a = nodes[m]->GetComponent<StaticModel>();
		StaticModel *b = nodes[n]->GetComponent<StaticModel>();

		Material *t = a->GetMaterial();
		a->SetMaterial(b->GetMaterial());
		b->SetMaterial(t);
	}
			
	

	for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
		Node *posterNode = *i;

		
			
		}
	}



