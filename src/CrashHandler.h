#pragma once

#include "Urho3D/Core/Object.h"

namespace google_breakpad {
    class ExceptionHandler;
}

class CrashHandler: public Urho3D::Object
{
    OBJECT(CrashHandler)

public:
    CrashHandler(Urho3D::Context *context);
    ~CrashHandler();

private:
    google_breakpad::ExceptionHandler *exceptionHandler_;
};
