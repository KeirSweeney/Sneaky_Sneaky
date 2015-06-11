#include "Urho3D/Urho3D.h"

#include "AnimatedPoster.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/Graphics/Texture2D.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Resource/XMLElement.h"
#include "Urho3D/Scene/Node.h"

using namespace Urho3D;

AnimatedPoster::AnimatedPoster(Context *context):
	InteractableComponent(context),
	speed_(1.0f), count_(1),
	timer_(0.0f), frame_(0)
{
}

void AnimatedPoster::RegisterObject(Context* context)
{
	context->RegisterFactory<AnimatedPoster>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void AnimatedPoster::LoadFromXML(const XMLElement &xml)
{
	speed_ = 1.0f / xml.GetFloat("speed");
	count_ = xml.GetInt("frames");
	path_ = xml.GetAttribute("path");
}

void AnimatedPoster::DelayedStart()
{
	StaticModel *model = node_->GetComponent<StaticModel>();

	Material *baseMaterial = model->GetMaterial();
	if (!baseMaterial) {
		LOGERROR("Bad material on animated poster.");
		return;
	}

	material_ = baseMaterial->Clone();

	// Bug in Urho3D that is easier to patch here than fork the engine again.
	material_->SetDepthBias(model->GetMaterial()->GetDepthBias());
	//TODO: Remove the previous line if Material::Clone() is fixed to copy depth bias params.

	model->SetMaterial(material_);
}

void AnimatedPoster::Update(float timeStep)
{
	if (material_.Null()) {
		// Can't remove in delayed start due to use-after-free.
		Remove();
		return;
	}

	timer_ -= timeStep;

	if (timer_ > 0.0f) {
		return;
	}

	timer_ = speed_;

	frame_++;

	if (frame_ >= count_) {
		frame_ = 0;
	}

	ResourceCache *cache = GetSubsystem<ResourceCache>();
	material_->SetTexture(TU_DIFFUSE, cache->GetResource<Texture2D>("Textures/" + path_ + "/" + String(frame_ + 1) + ".png"));
}
