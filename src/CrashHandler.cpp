#include "CrashHandler.h"

#include "Analytics.h"

#ifdef GOOGLE_BREAKPAD
#ifdef __APPLE__
#include "client/mac/handler/exception_handler.h"
#else //__APPLE__
#include "client/windows/handler/exception_handler.h"
#endif //__APPLE__

using namespace Urho3D;

#ifdef __APPLE__
bool BreakpadFilterCallback(void *context)
{
    (void)context;
    return true;
}

bool BreakpadMinidumpCallback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
{
    (void)context;
    fprintf(stderr, "Crash dump written to '%s/%s.dmp'.\n", dump_dir, minidump_id);

    Analytics *analytics = ((CrashHandler *)context)->GetSubsystem<Analytics>();
    analytics->SendCrashEvent(String(minidump_id));
    analytics->FlushEvents();

    return succeeded;
}
#else //__APPLE__
bool BreakpadFilterCallback(void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion)
{
    (void)context; (void)exinfo; (void)assertion;
    return true;
}

bool BreakpadMinidumpCallback(const wchar_t *dump_path, const wchar_t *minidump_id, void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion, bool succeeded)
{
    (void)context; (void)exinfo; (void)assertion;
    fprintf(stderr, "Crash dump written to '%ws/%ws.dmp'.\n", dump_path, minidump_id);

    Analytics *analytics = ((CrashHandler *)context)->GetSubsystem<Analytics>();
    analytics->SendCrashEvent(String(minidump_id));
    analytics->FlushEvents();

    return succeeded;
}
#endif //__APPLE__
#endif //GOOGLE_BREAKPAD

CrashHandler::CrashHandler(Context *context):
    Object(context), exceptionHandler_(NULL)
{
#ifdef GOOGLE_BREAKPAD
#ifdef __APPLE__
    exceptionHandler_ = new google_breakpad::ExceptionHandler(".", BreakpadFilterCallback, BreakpadMinidumpCallback, this, true, NULL);
#else
    exceptionHandler_ = new google_breakpad::ExceptionHandler(L".", BreakpadFilterCallback, BreakpadMinidumpCallback, this, google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif
#endif
}

CrashHandler::~CrashHandler()
{
#ifdef GOOGLE_BREAKPAD
    if (exceptionHandler_) {
        delete exceptionHandler_;
    }
#endif
}
