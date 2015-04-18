#include "Padlock.h"

#include "Inventory.h"
#include "Person.h"
#include "Pickup.h"

#include "Audio.h"
#include "Camera.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Graphics.h"
#include "Log.h"
#include "NavigationMesh.h"
#include "Node.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sound.h"
#include "SoundSource3D.h"
#include "Sprite.h"
#include "StaticModel.h"
#include "Text.h"
#include "UI.h"
#include "UIElement.h"

using namespace Urho3D;

const float Padlock::DISTANCE = 1.0f;

Padlock::Padlock(Context *context):
	InteractableComponent(context),
	displayWidth_(200), displayHeight_(100), content_("Lemons.")
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

	panel_ = ui->GetRoot()->CreateChild<UIElement>();
	panel_->SetFixedSize(displayWidth_, displayHeight_);
	panel_->SetVisible(false);

	Sprite *background = panel_->CreateChild<Sprite>();
	background->SetFixedSize(panel_->GetSize());
	background->SetColor(Color::BLACK);
	background->SetOpacity(0.6f);

	Text *label = panel_->CreateChild<Text>();
	label->SetFixedSize(panel_->GetSize() - IntVector2(20, 20));
	label->SetFont("Fonts/Anonymous Pro.ttf");
	label->SetColor(Color::WHITE);
	label->SetWordwrap(true);
	label->SetText(content_);
	label->SetAlignment(HA_CENTER, VA_CENTER);
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
