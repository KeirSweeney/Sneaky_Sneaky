#include "Padlock.h"
#include "Pickup.h"
#include "Inventory.h"

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
#include "Log.h"
#include "Profiler.h"
#include "StaticModel.h"

using namespace Urho3D;

const float Padlock::VIEW_DISTANCE = 2.0f;
const float Padlock::VIEW_ANGLE = 90.0f;

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
    Inventory *inv = personNode->GetComponent<Inventory>();

    const Vector<SharedPtr<Pickup>> &items = inv->GetItems();

    if(items.Empty()) {
        LOGERROR("ITEMS IS EMPTY");
    }

    for (Vector<SharedPtr<Pickup>>::ConstIterator i = items.Begin(); i != items.End(); ++i) {
        Urho3D::SharedPtr<Pickup> item = *i;

        if(item->GetPickupType() == content_) {
            StaticModel *model = node_->GetComponent<StaticModel>();
            RigidBody *rigidNode = node_->GetComponent<RigidBody>();
            model->SetEnabled(false);
            rigidNode->SetEnabled(false);
            panel_->SetVisible(false);
        }
  }

}

bool Padlock::CanPlayerInteract()
{
    Vector3 PadlockPosition = node_->GetWorldPosition();
    Vector3 personPosition = GetScene()->GetChild("Person", true)->GetWorldPosition();
    Vector3 difference = personPosition - PadlockPosition;

    if(difference.LengthSquared() > (VIEW_DISTANCE * VIEW_DISTANCE)) {
        return false;
    }

    Vector3 forward = -node_->GetWorldDirection();
    difference.Normalize();

    //DebugRenderer *debug = node_->GetScene()->GetComponent<DebugRenderer>();
    //debug->AddLine(PadlockPosition, PadlockPosition + forward, Color::BLUE);
    //debug->AddLine(PadlockPosition, PadlockPosition + difference, Color::RED);

    if (forward.DotProduct(difference) < Cos(VIEW_ANGLE / 2.0f)) {
        return false;
    }

    return true;
}
