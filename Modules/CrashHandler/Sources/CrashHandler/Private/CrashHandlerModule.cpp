#include "CrashHandler/CrashHandlerModule.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>

#if defined(__DAVAENGINE_LINUX__)
#include <breakpad/client/linux/handler/exception_handler.h>
#endif

namespace DAVA
{
static bool CrashDumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    printf("Crash dump written to: %s\n", descriptor.path());
    return succeeded;
}

google_breakpad::ExceptionHandler* exceptionHandler = nullptr;

bool InstallCrashHandler(const char* crashDumpDir)
{
    auto r = GetEngineContext()->fileSystem->CreateDirectory(crashDumpDir);
    if (r == FileSystem::DIRECTORY_CREATED || r == FileSystem::DIRECTORY_EXISTS)
    {
        google_breakpad::MinidumpDescriptor descriptor(crashDumpDir);
        exceptionHandler = new google_breakpad::ExceptionHandler(descriptor, nullptr, CrashDumpCallback, nullptr, true, -1);
        return true;
    }

    Logger::Error("Crash handler is not installed: crash dump directory is not created");
    return false;
}

} // namespace DAVA
