#include "Urho3D/Urho3D.h"

#include "Stairs.h"

#include "Game.h"

#include "Urho3D/Core/Context.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Graphics/Light.h"
#include "Urho3D/IO/Log.h"
#include "Urho3D/Navigation/Navigable.h"
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Scene/Scene.h"

using namespace Urho3D;

Stairs::Stairs(Context *context):
	InteractableComponent(context)
{
}

void Stairs::RegisterObject(Context* context)
{
	context->RegisterFactory<Stairs>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}

void Stairs::DelayedStart()
{
	node_->CreateComponent<Navigable>();

	// Create a number of weak lights up the center of the staircase to provide some extra lighting.
	Node *lightNode = node_->CreateChild("StairLight");
	lightNode->SetPosition(Vector3(0.0f, 6.0f, 0.0f));
	lightNode->SetDirection(Vector3::DOWN);

	Light *light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_POINT);
	light->SetBrightness(0.2f);
	light->SetRange(9.0f);
	light->SetColor(Color::WHITE);

	lightNode = lightNode->Clone();
	lightNode->Translate(Vector3(0.0f, 3.0f, 0.0f), TS_PARENT);

	lightNode = lightNode->Clone();
	lightNode->Translate(Vector3(0.0f, 3.0f, 0.0f), TS_PARENT);

	lightNode = lightNode->Clone();
	lightNode->Translate(Vector3(0.0f, 3.0f, 0.0f), TS_PARENT);

	lightNode = lightNode->Clone();
	lightNode->Translate(Vector3(0.0f, 3.0f, 0.0f), TS_PARENT);
}

void Stairs::Update(float timeStep)
{
	Vector3 position = node_->GetWorldPosition();
	Vector3 personPosition = GetScene()->GetChild("Person", true)->GetWorldPosition();
	Vector3 personOffset = position - personPosition;

	if (personOffset.LengthSquared() > (5.0f * 5.0f)) {
		return;
	}

	GetSubsystem<Game>()->EndLevel(false, false);
}
