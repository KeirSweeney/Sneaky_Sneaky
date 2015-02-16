#include "Inventory.h"

#include "Pickup.h"

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
#include "Input.h"

using namespace Urho3D;

const int Inventory::PADDING = 20;

Inventory::Inventory(Context *context):
    LogicComponent(context)
{
}

void Inventory::RegisterObject(Context* context)
{
    context->RegisterFactory<Inventory>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void Inventory::DelayedStart()
{
    UI *ui = GetSubsystem<UI>();

    UIElement *panel = ui->GetRoot()->CreateChild<UIElement>();
    panel->SetFixedSize(panel->GetParent()->GetSize() - IntVector2(PADDING * 2, PADDING * 2));
    panel->SetAlignment(HA_CENTER, VA_CENTER);
    panel->SetVisible(false);

    Sprite *background = panel->CreateChild<Sprite>();
    background->SetFixedSize(panel->GetSize());
    background->SetColor(Color::BLACK);
    background->SetOpacity(0.6f);

    panel_ = panel->CreateChild<UIElement>();
    panel_->SetFixedSize(panel->GetSize());
}

void Inventory::Update(float timeStep)
{
    Input *input = GetSubsystem<Input>();

    if (!input->GetKeyDown(KEY_TAB)) {
        panel_->GetParent()->SetVisible(false);
        return;
    }

    // If we're marked as dirty, remove all the UI children
    // and recreate them from the current inventory contents.
    if (dirty_) {
        panel_->RemoveAllChildren();

        int x = PADDING;
        int y = PADDING;
        for (Vector<SharedPtr<Pickup>>::ConstIterator i = items_.Begin(); i != items_.End(); ++i) {
            UIElement *item = panel_->CreateChild<UIElement>();
            item->SetFixedSize(200, 200);
            item->SetPosition(x, y);

            Sprite *background = item->CreateChild<Sprite>();
            background->SetFixedSize(item->GetSize());
            background->SetColor(Color::WHITE);
            background->SetOpacity(0.2f);

            Text *label = item->CreateChild<Text>();
            label->SetFont("Fonts/Anonymous Pro.ttf");
            label->SetColor(Color::WHITE);
            label->SetText((*i)->GetPickupType());
            label->SetAlignment(HA_CENTER, VA_CENTER);

            x += item->GetWidth() + PADDING;
            if (x > panel_->GetWidth() - PADDING) {
                x = 0;
                y += item->GetHeight() + PADDING;
            }
        }
    }

    panel_->GetParent()->SetVisible(true);
}

void Inventory::AddItem(Pickup *item)
{
    items_.Push(SharedPtr<Pickup>(item));
    dirty_ = true;
}

int Inventory::GetItemCount()
{
    return items_.Size();
}

SharedPtr<Pickup> Inventory::GetThrowableItem()
{
    Urho3D::SharedPtr<Pickup> returnItem;

    for (Vector<SharedPtr<Pickup>>::ConstIterator i = items_.Begin(); i != items_.End(); ++i) {
        Urho3D::SharedPtr<Pickup> item = *i;
        if(item->GetPickupType() == "Pickup")
        {
            returnItem = item;
            break;
        }
    }

    if(returnItem.NotNull())
    {
        items_.Remove(returnItem);
    }

    return returnItem;
}
