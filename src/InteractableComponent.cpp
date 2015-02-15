#include "InteractableComponent.h"

#include "Context.h"

using namespace Urho3D;

InteractableComponent::InteractableComponent(Context *context):
    LogicComponent(context)
{
}

void InteractableComponent::RegisterObject(Context* context)
{
    context->RegisterFactory<InteractableComponent>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}

void InteractableComponent::LoadFromXML(const XMLElement &xml)
{
    // Urho3D doesn't like abstract classes.
}
