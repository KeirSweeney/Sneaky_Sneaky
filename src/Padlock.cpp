#include "Urho3D/Urho3D.h"

#include "Padlock.h"

#include "Inventory.h"
#include "Person.h"
#include "Pickup.h"

#include "Urho3D/Audio/Audio.h"
#include "Urho3D/Audio/Sound.h"
#include "Urho3D/Audio/SoundSource3D.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/Graphics/StaticModel.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/UI/UIElement.h"

using namespace Urho3D;

const float Padlock::DISTANCE = 1.0f;

Padlock::Padlock(Context *context):
	InteractableComponent(context),
	displayWidth_(200), displayHeight_(100), panel_(NULL), content_("Lemons.")
{
}

void Padlock::RegisterObject(Context* context)
{
	context->RegisterFactory<Padlock>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void Padlock::LoadFromXML(const XMLElement &xml)
{
	if (xml.HasAttribute("displaysize")) {
		IntVector2 displaySize = xml.GetIntVector2("displaysize");
		displayWidth_ = displaySize.x_;
		displayHeight_ = displaySize.y_;
	}

	const String content = xml.GetValue();
	if (!content.Empty()) {
		content_ = content;
	}

	type_ = xml.HasAttribute("type") ? xml.GetAttribute("type") : content_;
}

void Padlock::DelayedStart()
{
	UI *ui = GetSubsystem<UI>();
	float pixelRatio = GetSubsystem<Graphics>()->GetPixelRatio();

	panel_ = ui->GetRoot()->CreateChild<UIElement>();
	panel_->SetFixedSize(displayWidth_ * pixelRatio, displayHeight_ * pixelRatio);
	panel_->SetVisible(false);

	Sprite *background = panel_->CreateChild<Sprite>();
	background->SetFixedSize(panel_->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.6f);

	Text *label = panel_->CreateChild<Text>();
	label->SetFixedSize(panel_->GetSize() - (IntVector2(20, 20) * pixelRatio));
	label->SetFont("Fonts/Anonymous Pro.ttf");
	label->SetColor(Color::WHITE);
	label->SetWordwrap(true);
	label->SetText(content_);
	label->SetTextAlignment(HA_CENTER);
	label->SetAlignment(HA_CENTER, VA_CENTER);
}

void Padlock::Stop()
{
	if (panel_) {
		panel_->Remove();
		panel_ = NULL;
	}
}

void Padlock::Update(float timeStep)
{
	Vector3 position = node_->GetWorldPosition();

	if (!CanPlayerInteract()) {
		panel_->SetVisible(false);
		return;
	}

	Renderer *renderer = GetSubsystem<Renderer>();
	Camera *camera = renderer->GetViewport(0)->GetCamera();
	Vector2 screenPosition = camera->WorldToScreenPoint(position);

	Graphics *graphics = GetSubsystem<Graphics>();
	IntVector2 uiPosition;
	uiPosition.x_ = (int)round((screenPosition.x_ * graphics->GetWidth()) - (panel_->GetWidth() / 2.0f));
	uiPosition.y_ = (int)round((screenPosition.y_ * graphics->GetHeight()) - (panel_->GetHeight() / 2.0f));

	panel_->SetPosition(uiPosition);
	panel_->SetVisible(true);

	Node *personNode = GetScene()->GetChild("Person", true);
	Inventory *inventory = personNode->GetComponent<Inventory>();

	SharedPtr<Pickup> item = inventory->GetItemOfType(type_, false);

	if (item.Null()) {
		return;
	}

	ResourceCache *cache = GetSubsystem<ResourceCache>();

	Node *sound = GetScene()->CreateChild("SoundEffect");
	sound->SetWorldPosition(node_->GetWorldPosition());

	SoundSource *source = sound->CreateComponent<SoundSource>();
	source->SetAutoRemove(true);
	source->SetSoundType(SOUND_EFFECT);
	source->Play(cache->GetResource<Sound>("Audio/Unlock.ogg"));

	panel_->SetVisible(false);

	node_->Remove();
}

bool Padlock::CanPlayerInteract()
{
	Vector3 PadlockPosition = node_->GetWorldPosition();
	Vector3 personPosition = GetScene()->GetChild("Person", true)->GetWorldPosition();
	Vector3 difference = personPosition - PadlockPosition;

	if(difference.LengthSquared() > (DISTANCE * DISTANCE)) {
		return false;
	}

	return true;
}
