#pragma once

#include <Entity/SingleComponent.h>

namespace DAVA
{
class NetworkResimulationSingleComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSingleComponent, SingleComponent);

public:
    void SetResimulationFrameId(uint32 frameId);
    uint32 GetResimulationFrameId() const;

private:
    uint32 resimulationFrameId = 0;
};
} // namespace DAVA