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

using namespace Urho3D;

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
    Vector3 terminalPosition = node_->GetWorldPosition();
    Vector3 personPosition = GetScene()->GetChild("Person", true)->GetPosition();
    Vector3 personOffset = terminalPosition - personPosition;

    if (personOffset.LengthSquared() > (2.0f * 2.0f)) {
        panel_->SetVisible(false);
        return;
    }

    Camera *camera = GetScene()->GetChild("Camera", true)->GetComponent<Camera>();
    Vector2 screenPosition = camera->WorldToScreenPoint(terminalPosition);

    Graphics *graphics = GetSubsystem<Graphics>();
    IntVector2 uiPosition;
    uiPosition.x_ = (int)round((screenPosition.x_ * graphics->GetWidth()) - (panel_->GetWidth() / 2.0f));
    uiPosition.y_ = (int)round((screenPosition.y_ * graphics->GetHeight()) - (panel_->GetHeight() / 2.0f));

    panel_->SetPosition(uiPosition);
    panel_->SetVisible(true);
}
