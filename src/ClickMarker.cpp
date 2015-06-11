#include "Urho3D/Urho3D.h"

#include "ClickMarker.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Material.h"
#include "Urho3D/Graphics/Model.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"

using namespace Urho3D;

ClickMarker::ClickMarker(Context *context) :
	LogicComponent(context)
{
}

void ClickMarker::RegisterObject(Context* context)
{
	context->RegisterFactory<ClickMarker>("Logic");
	COPY_BASE_ATTRIBUTES(LogicComponent);
}

void ClickMarker::Start()
{
	ResourceCache *cache = GetSubsystem<ResourceCache>();

	material_ = cache->GetResource<Material>("Materials/Marker.xml")->Clone();

	StaticModel *markerModel = node_->CreateComponent<StaticModel>();
	markerModel->SetModel(cache->GetResource<Model>("Models/Marker.mdl"));
	markerModel->SetMaterial(material_);
	markerModel->SetViewMask(0x02);

	node_->SetScale(0.1f);
}

void ClickMarker::Update(float timeStep)
{
	node_->Rotate(Quaternion(360.0f * timeStep, Vector3::UP));
	node_->Scale(1.0f + (2.5f * timeStep));

	Vector4 color = material_->GetShaderParameter("MatDiffColor").GetVector4();
	color.w_ -= 1.5f * timeStep;
	material_->SetShaderParameter("MatDiffColor", color);

	if (color.w_ <= 0.0f) {
		node_->Remove();
	}
}

