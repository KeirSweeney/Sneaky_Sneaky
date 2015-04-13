#include "InteractablePoster.h"

#include "Context.h"

using namespace Urho3D;

InteractablePoster::InteractablePoster(Context *context) :
    LogicComponent(context)
{
}

void InteractablePoster::RegisterObject(Context* context)
{
	context->RegisterFactory<InteractablePoster>("Logic");

    COPY_BASE_ATTRIBUTES(LogicComponent);
}


