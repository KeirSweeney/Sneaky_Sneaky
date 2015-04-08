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
	Node *roomNode = node_->GetParent();
	PODVector<Node *> nodes;

	roomNode->GetChildrenWithComponent<InteractablePoster>(nodes, true);

	ResourceCache *cache = GetSubsystem<ResourceCache>();

	for (PODVector<Node *>::ConstIterator i = nodes.Begin(); i != nodes.End(); ++i) {
		Node *posterNode = *i;

		StaticModel *glyphModel = posterNode->GetComponent<StaticModel>();
				
	}		

		//StaticModel *glyphModel = glyphs_[i] = glyphNode->CreateComponent<StaticModel>();
		//glyphModel->SetModel(cache->GetResource<Model>("PersonPlane.mdl"));
		//glyphModel->SetMaterial(cache->GetResource<Material>("Materials/Glyph" + String(i) + ".xml"));
	

}

void MrWright::DelayedStart()
{
 
}

void MrWright::Update(float timeStep)
{
	/*swapTimer_ += timeStep;

	if (swapTimer_ < 5.0f) {
		return;
	}
	
	swapTimer_ = 0.0f;

	for (int i = 0; i < (6 - 1); ++i) {
		int n = i + Rand() / (32767 / (6 - i) + 1);
		Material *temp = glyphs_[n]->GetMaterial();
		glyphs_[n]->SetMaterial(glyphs_[i]->GetMaterial());
		glyphs_[i]->SetMaterial(temp);
	} */
}

