#include "Modules/RenderOptionsModule/Private/RenderOptionsData.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Renderer.h>

DAVA_VIRTUAL_REFLECTION_IMPL(RenderOptionsData)
{
    DAVA::ReflectionRegistrator<RenderOptionsData>::Begin()
    .Field("isEnabled", &RenderOptionsData::IsEnabled, nullptr)
    .End();
}

bool RenderOptionsData::IsEnabled() const
{
    return DAVA::Renderer::IsInitialized();
}
