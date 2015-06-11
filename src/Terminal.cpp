#include "Urho3D/Urho3D.h"

#include "Terminal.h"

#include "Person.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Core/Profiler.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/UI/UIElement.h"

using namespace Urho3D;

const float Terminal::VIEW_DISTANCE = 2.0f;
const float Terminal::VIEW_ANGLE = 90.0f;

Terminal::Terminal(Context *context):
	InteractableComponent(context),
	displayWidth_(200), displayHeight_(100), content_("Lemons.")
{
}

void Terminal::RegisterObject(Context* context)
{
	context->RegisterFactory<Terminal>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void Terminal::LoadFromXML(const XMLElement &xml)
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
}

void Terminal::DelayedStart()
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
	label->SetAlignment(HA_CENTER, VA_CENTER);
}

void Terminal::Update(float timeStep)
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
}

bool Terminal::CanPlayerInteract()
{
	Vector3 terminalPosition = node_->GetWorldPosition();
	Vector3 personPosition = GetScene()->GetChild("Person", true)->GetWorldPosition();

	Vector3 difference = personPosition - terminalPosition;
	difference.y_ = 0.0f;

	if(difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE)) {
		return false;
	}

	Vector3 forward = -node_->GetWorldDirection();
	difference.Normalize();

#if 0
	DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
	debug->AddLine(terminalPosition, terminalPosition + forward, Color::BLUE);
	debug->AddLine(terminalPosition, terminalPosition + difference, Color::RED);
#endif

	if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
		return false;
	}

	return true;
}
