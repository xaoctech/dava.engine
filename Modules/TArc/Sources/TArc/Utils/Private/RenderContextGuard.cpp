#include "TArc/Utils/RenderContextGuard.h"

#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Window.h>

namespace DAVA
{
namespace TArc
{
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
} // namespace TArc
} // namespace DAVA
