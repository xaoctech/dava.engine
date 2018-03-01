#pragma once

#include <Entity/SingletonComponent.h>

namespace DAVA
{
class NetworkResimulationSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkResimulationSingleComponent, SingletonComponent);

public:
    void SetResimulationFrameId(uint32 frameId);
    uint32 GetResimulationFrameId() const;

private:
    void Clear() override;

    uint32 resimulationFrameId = 0;
};
} // namespace DAVA