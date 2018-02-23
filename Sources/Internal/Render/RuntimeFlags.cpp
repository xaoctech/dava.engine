#include "RuntimeFlags.h"

namespace DAVA
{
void RuntimeFlags::SetFlag(Flag flag, int32 value)
{
    values[flag].value = value;
    values[flag].wasSet = true;
}

void RuntimeFlags::ResetFlag(Flag flag)
{
    values[flag].value = 0;
    values[flag].wasSet = false;
}

void RuntimeFlags::ResetFlags()
{
    for (FlagValue& value : values)
    {
        value.value = 0;
        value.wasSet = false;
    }
}

int32 RuntimeFlags::GetFlagValue(Flag flag) const
{
    DVASSERT(values[flag].wasSet);
    return values[flag].value;
}

const FastName& RuntimeFlags::GetNameForFlag(Flag flag) const
{
    static const FastName names[FLAGS_COUNT] =
    {
      FastName("SHADOW_CASCADES"),
      FastName("SHADOW_PCF"),
      FastName("ATMOSPHERE_SCATTERING_SAMPLES"),
    };
    return names[flag];
}

void RuntimeFlags::AddToDefines(UnorderedMap<FastName, int32>& defines) const
{
    for (uint32 i = 0; i < FLAGS_COUNT; ++i)
    {
        if (values[i].wasSet)
        {
            defines[GetNameForFlag(static_cast<Flag>(i))] = values[i].value;
        }
    }
}
}
