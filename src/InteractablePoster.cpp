#include "Urho3D/Urho3D.h"

#include "InteractablePoster.h"

#include "Urho3D/Core/Context.h"

using namespace Urho3D;

InteractablePoster::InteractablePoster(Context *context) :
	InteractableComponent(context)
{
}

void InteractablePoster::RegisterObject(Context* context)
{
	context->RegisterFactory<InteractablePoster>("Logic");

	COPY_BASE_ATTRIBUTES(InteractableComponent);
}


