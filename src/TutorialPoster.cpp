#include "Urho3D/Urho3D.h"

#include "TutorialPoster.h"

#include "Game.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/Graphics/Texture2D.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/XMLElement.h"
#include "Urho3D/Scene/Node.h"

using namespace Urho3D;

TutorialPoster::TutorialPoster(Context *context):
	InteractableComponent(context)
{
}

void TutorialPoster::RegisterObject(Context* context)
{
	context->RegisterFactory<TutorialPoster>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void TutorialPoster::LoadFromXML(const XMLElement &xml)
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();
	controllerMaterial_ = cache->GetResource<Material>("Materials/" + xml.GetAttribute("controller_material") + ".xml");
}

void TutorialPoster::DelayedStart()
{
	StaticModel *model = node_->GetComponent<StaticModel>();
	material_ = model->GetMaterial();

	if (controllerMaterial_.Null()) {
		controllerMaterial_ = material_;
	}
}

void TutorialPoster::Update(float timeStep)
{
	StaticModel *model = node_->GetComponent<StaticModel>();

	if (GetSubsystem<Game>()->GetCurrentJoystick() == -1) {
		model->SetMaterial(material_);
	} else {
		model->SetMaterial(controllerMaterial_);
	}
}
