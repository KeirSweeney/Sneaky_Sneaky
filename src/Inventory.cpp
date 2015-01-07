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
bool myPickup;

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

    panel_ = ui->GetRoot()->CreateChild<UIElement>();
    panel_->SetFixedSize(panel_->GetParent()->GetSize() - IntVector2(100, 100));
    panel_->SetAlignment(HA_CENTER, VA_CENTER);
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

void Inventory::Update(float timeStep)
{
    Input *input = GetSubsystem<Input>();

    if (!input->GetKeyDown(KEY_TAB)) {
        panel_->SetVisible(false);
        return;
    }

    panel_->SetVisible(true);

	if (myPickup == true)
	{
		Text *label = panel_->CreateChild<Text>();
		label->SetFont("Fonts/Anonymous Pro.ttf");
		label->SetColor(Color::WHITE);
		label->SetText("Tape");
		label->SetAlignment(HA_LEFT, VA_CENTER);
		//would be nice to add the object into the inventory rather than just text.
	}
	
}

void Inventory::AddToInventory(bool itemAdded)
{
	myPickup = itemAdded;
}
