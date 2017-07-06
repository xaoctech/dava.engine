#pragma once

#include <Functional/Function.h>
#include <Base/BaseTypes.h>

class FrameUpdater
{
public:
    using Updater = DAVA::Function<void()>;

    FrameUpdater();

    void SetUpdater(const Updater& updater);

    void Update();

private:
    void OnUpdate(DAVA::float32);

    Updater updater;
    bool needUpdate = false;
};
