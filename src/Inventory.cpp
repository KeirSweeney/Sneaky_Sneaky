#include "Urho3D/Urho3D.h"

#include "Inventory.h"

#include "Person.h"
#include "Pickup.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Navigation/NavigationMesh.h"
#include "Urho3D/Physics/RigidBody.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/UI/UIElement.h"

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
	float pixelRatio = GetSubsystem<Graphics>()->GetPixelRatio();

	UIElement *panel = ui->GetRoot()->CreateChild<UIElement>();
	panel->SetFixedSize(panel->GetParent()->GetSize() - (IntVector2(PADDING * 2, PADDING * 2) * pixelRatio));
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

	bool tabDown = false;

	if (input->GetNumJoysticks() > 0) {
		JoystickState *state = input->GetJoystickByIndex(0);

		tabDown = state->GetButtonDown(3);
	}

	tabDown = tabDown || input->GetKeyDown(KEY_TAB);

	if (!tabDown) {
		panel_->GetParent()->SetVisible(false);
		return;
	}

	// If we're marked as dirty, remove all the UI children
	// and recreate them from the current inventory contents.
	if (dirty_) {
		panel_->RemoveAllChildren();
		
		float pixelRatio = GetSubsystem<Graphics>()->GetPixelRatio();

		int x = PADDING * pixelRatio;
		int y = PADDING * pixelRatio;
		int w = (panel_->GetWidth() - (PADDING * 6 * pixelRatio)) / 5;
		for (Vector<SharedPtr<Pickup>>::ConstIterator i = items_.Begin(); i != items_.End(); ++i) {
			UIElement *item = panel_->CreateChild<UIElement>();
			item->SetFixedSize(w, w);
			item->SetPosition(x, y);

			Sprite *background = item->CreateChild<Sprite>();
			background->SetFixedSize(item->GetSize());
			background->SetColor(Color::WHITE);
			background->SetOpacity(0.2f);

			Text *label = item->CreateChild<Text>();
			label->SetFixedSize(item->GetSize() - (IntVector2(PADDING, PADDING) * pixelRatio));
			label->SetFont("Fonts/Anonymous Pro.ttf");
			label->SetColor(Color::WHITE);
			label->SetText((*i)->GetDisplayName());
			label->SetAlignment(HA_CENTER, VA_CENTER);
			label->SetTextAlignment(HA_CENTER);
			label->SetWordwrap(true);

			x += w + (PADDING * pixelRatio);
			if (x >= panel_->GetWidth() - (PADDING * pixelRatio)) {
				x = PADDING * pixelRatio;
				y += w + (PADDING * pixelRatio);
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
