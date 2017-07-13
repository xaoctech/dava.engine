#include "Modules/RenderOptionsModule/Private/OptionWrapper.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Renderer.h>

DAVA_VIRTUAL_REFLECTION_IMPL(OptionWrapper)
{
    using namespace DAVA;
    ReflectionRegistrator<OptionWrapper>::Begin()
    .Field("title", &OptionWrapper::GetTitle, nullptr)
    .Field("enabled", &OptionWrapper::GetEnabled, &OptionWrapper::SetEnabled)
    .End();
}

OptionWrapper::OptionWrapper(RenderOptions::RenderOption option)
    : option(option)
{
}

String OptionWrapper::GetTitle() const
{
    using namespace DAVA;
    RenderOptions* options = Renderer::GetOptions();
    if (options)
    {
        return String(options->GetOptionName(option).c_str());
    }
    return String();
}

bool OptionWrapper::GetEnabled() const
{
    using namespace DAVA;
    RenderOptions* options = Renderer::GetOptions();
    if (options)
    {
        return options->IsOptionEnabled(option);
    }
    return false;
}

void OptionWrapper::SetEnabled(bool value)
{
    using namespace DAVA;
    RenderOptions* options = Renderer::GetOptions();
    if (options)
    {
        options->SetOption(option, value);
    }
}