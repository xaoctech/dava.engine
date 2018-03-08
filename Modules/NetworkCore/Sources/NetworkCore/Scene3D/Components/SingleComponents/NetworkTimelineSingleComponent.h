#pragma once

#include <Entity/SingleComponent.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NetworkTimelineSingleComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkTimelineSingleComponent, SingleComponent);

public:
    void SetClientJustPaused(bool value);
    bool IsClientJustPaused() const;

    void SetServerJustPaused(bool value);
    bool IsServerJustPaused() const;

    void SetStepOver(bool value);
    bool HasStepOver() const;

    void SetStepsCount(int32 value);
    int32 GetStepsCount() const;

private:
    bool clientJustPaused = false;
    bool serverJustPaused = false;
    bool stepOver = false;
    int32 stepsCount = 0;
};
}
