#include "Inventory.h"

#include "Pickup.h"
#include "Person.h"

#include "Camera.h"
#include "Context.h"
#include "DebugRenderer.h"
#include "Graphics.h"
#include "Input.h"
#include "Log.h"
#include "NavigationMesh.h"
#include "Node.h"
#include "RigidBody.h"
#include "Scene.h"
#include "Sprite.h"
#include "Text.h"
#include "UI.h"
#include "UIElement.h"

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
			item->SetFixedSize(100, 100);
			item->SetPosition(x, y);

			Sprite *background = item->CreateChild<Sprite>();
			background->SetFixedSize(item->GetSize());
			background->SetColor(Color::WHITE);
			background->SetOpacity(0.2f);

			Text *label = item->CreateChild<Text>();
			label->SetFixedSize(item->GetSize() - IntVector2(PADDING, PADDING));
			label->SetFont("Fonts/Anonymous Pro.ttf");
			label->SetColor(Color::WHITE);
			label->SetText((*i)->GetDisplayName());
			label->SetAlignment(HA_CENTER, VA_CENTER);
			label->SetTextAlignment(HA_CENTER);
			label->SetWordwrap(true);

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

SharedPtr<Pickup> Inventory::GetItemOfType(String type, bool remove)
{
	Urho3D::SharedPtr<Pickup> returnItem;

	for (Vector<SharedPtr<Pickup>>::ConstIterator i = items_.Begin(); i != items_.End(); ++i) {
		Urho3D::SharedPtr<Pickup> item = *i;
		if(item->GetPickupType().Compare(type, false) == 0) {
			returnItem = item;
			break;
		}
	}

	if(remove && returnItem.NotNull()) {
		items_.Remove(returnItem);
	}

	return returnItem;
}
