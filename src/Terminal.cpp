#include "Terminal.h"

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

const float Terminal::VIEW_DISTANCE = 1.5f;
const float Terminal::VIEW_ANGLE = 90.0f;

Terminal::Terminal(Context *context):
    LogicComponent(context)
{
}

void Terminal::RegisterObject(Context* context)
{
    context->RegisterFactory<Terminal>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Terminal::DelayedStart()
{
    UI *ui = GetSubsystem<UI>();

    panel_ = ui->GetRoot()->CreateChild<UIElement>();
    panel_->SetFixedSize(200, 100);
    panel_->SetVisible(false);

    Sprite *background = panel_->CreateChild<Sprite>();
    background->SetFixedSize(panel_->GetSize());
    background->SetColor(Color::BLACK);
    background->SetOpacity(0.6f);

    Text *label = panel_->CreateChild<Text>();
    label->SetFont("Fonts/Anonymous Pro.ttf");
    label->SetColor(Color::WHITE);
    label->SetText("Lemons.");
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

    if(difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE))
    {
        return false;
    }

    Vector3 forward = -node_->GetWorldDirection();
    difference.Normalize();

    //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
    //debug->AddLine(terminalPosition, terminalPosition + forward, Color::BLUE);
    //debug->AddLine(terminalPosition, terminalPosition + difference, Color::RED);

    if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
        return false;
    }

    return true;
}
