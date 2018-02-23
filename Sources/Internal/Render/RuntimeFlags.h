#pragma once

#include "Base/FastName.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
class RuntimeFlags
{
public:
    enum Flag
    {
        SHADOW_CASCADES,
        SHADOW_PCF,
        ATMOSPHERE_SCATTERING_SAMPLES,
        FLAGS_COUNT,
    };

    void SetFlag(Flag flag, int32 value);
    void ResetFlag(Flag flag);
    void ResetFlags();

    int32 GetFlagValue(Flag) const;

    void AddToDefines(UnorderedMap<FastName, int32>& defines) const;
    const FastName& GetNameForFlag(Flag flag) const;

private:
    struct FlagValue
    {
        int32 value = 0;
        bool wasSet = false;
    };
    FlagValue values[FLAGS_COUNT];
};
}
