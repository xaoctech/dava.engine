#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <Render/RenderOptions.h>

class OptionWrapper : public DAVA::ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION(OptionWrapper, DAVA::ReflectionBase);

public:
    OptionWrapper(RenderOptions::RenderOption option);

    String GetTitle() const;
    bool GetEnabled() const;
    void SetEnabled(bool value);

private:
    RenderOptions::RenderOption option;
};
