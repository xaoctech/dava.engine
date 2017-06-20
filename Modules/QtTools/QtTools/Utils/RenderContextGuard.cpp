#include "RenderContextGuard.h"

#include "Engine/Engine.h"
#include "Engine/Window.h"

RenderContextGuard::RenderContextGuard()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        DAVA::Window* window = engine->PrimaryWindow();
        DAVA::PlatformApi::Qt::AcquireWindowContext(window);
    }
}

RenderContextGuard::~RenderContextGuard()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        DAVA::Window* window = engine->PrimaryWindow();
        DAVA::PlatformApi::Qt::ReleaseWindowContext(window);
    }
}
