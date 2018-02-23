#include "TArc/Utils/RenderContextGuard.h"

#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Window.h>
#include <Concurrency/Thread.h>
#include <Render/RHI/rhi_Public.h>

namespace DAVA
{
RenderContextGuard::RenderContextGuard()
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    DVASSERT(Thread::IsMainThread() == true);
    bool singleThreadRender = rhi::GetInitializeParams().threadedRenderEnabled == false;
    if (!engine->IsConsoleMode() && singleThreadRender == true)
    {
        Window* window = engine->PrimaryWindow();
        PlatformApi::Qt::AcquireWindowContext(window);
    }
}

RenderContextGuard::~RenderContextGuard()
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    DVASSERT(Thread::IsMainThread() == true);
    bool singleThreadRender = rhi::GetInitializeParams().threadedRenderEnabled == false;
    if (!engine->IsConsoleMode() && singleThreadRender == true)
    {
        Window* window = engine->PrimaryWindow();
        PlatformApi::Qt::ReleaseWindowContext(window);
    }
}
} // namespace DAVA
