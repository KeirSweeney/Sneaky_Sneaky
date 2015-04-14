#include "MrWrightTerminal.h"

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

using namespace Urho3D;

const float MrWrightTerminal::VIEW_DISTANCE = 2.0f;
const float MrWrightTerminal::VIEW_ANGLE = 90.0f;

MrWrightTerminal::MrWrightTerminal(Context *context):
    InteractableComponent(context),
    displayWidth_(200), displayHeight_(100), content_("")
{
}

void MrWrightTerminal::RegisterObject(Context* context)
{
    context->RegisterFactory<MrWrightTerminal>("Logic");

    COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void MrWrightTerminal::LoadFromXML(const XMLElement &xml)
{
    if (xml.HasAttribute("displaysize")) {
        IntVector2 displaySize = xml.GetIntVector2("displaysize");
        displayWidth_ = displaySize.x_;
        displayHeight_ = displaySize.y_;
    }

    //const String content = xml.GetValue();
    //if (!content.Empty()) {
    //    content_ = content;
    //}
}

void MrWrightTerminal::SetContent(String str)
{
	LOGERROR("SET TEXT");
	content_ = str.CString();
}

void MrWrightTerminal::DelayedStart()
{
    UI *ui = GetSubsystem<UI>();

    panel_ = ui->GetRoot()->CreateChild<UIElement>();
    panel_->SetFixedSize(displayWidth_, displayHeight_);
    panel_->SetVisible(false);

    Sprite *background = panel_->CreateChild<Sprite>();
    background->SetFixedSize(panel_->GetSize());
    background->SetColor(Color::BLACK);
    background->SetOpacity(0.6f);

	label_ = panel_->CreateChild<Text>();
	label_->SetFixedSize(panel_->GetSize() - IntVector2(20, 20));
	label_->SetFont("Fonts/Anonymous Pro.ttf");
	label_->SetColor(Color::WHITE);
	label_->SetWordwrap(true);
	label_->SetText(content_);
	label_->SetAlignment(HA_CENTER, VA_CENTER);
}

void MrWrightTerminal::Update(float timeStep)
{
	label_->SetText(content_);

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

bool MrWrightTerminal::CanPlayerInteract()
{
    Vector3 MrWrightTerminalPosition = node_->GetWorldPosition();
    Vector3 personPosition = GetScene()->GetChild("Person", true)->GetWorldPosition();
    Vector3 difference = personPosition - MrWrightTerminalPosition;

    if(difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE)) {
        return false;
    }

    Vector3 forward = -node_->GetWorldDirection();
    difference.Normalize();

    //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
    //debug->AddLine(MrWrightTerminalPosition, MrWrightTerminalPosition + forward, Color::BLUE);
    //debug->AddLine(MrWrightTerminalPosition, MrWrightTerminalPosition + difference, Color::RED);

    if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
        return false;
    }

    return true;
}
