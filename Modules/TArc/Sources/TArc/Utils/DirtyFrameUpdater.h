#pragma once

#include <Functional/Function.h>
#include <Base/BaseTypes.h>

class DirtyFrameUpdater
{
public:
    using Callback = DAVA::Function<void()>;

    DirtyFrameUpdater();

    void SetCallback(const Callback& callback);

    void MarkDirty();

private:
    void OnUpdate(DAVA::float32);

    Callback callback;
    bool isDirty = false;
};
