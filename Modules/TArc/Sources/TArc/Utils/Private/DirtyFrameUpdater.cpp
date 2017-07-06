#include "TArc/Utils/DirtyFrameUpdater.h"

#include <Engine/Engine.h>

DirtyFrameUpdater::DirtyFrameUpdater()
{
    DAVA::Engine::Instance()->update.Connect(this, &DirtyFrameUpdater::OnUpdate);
}

void DirtyFrameUpdater::SetCallback(const Callback& callback_)
{
    callback = callback_;
}

void DirtyFrameUpdater::MarkDirty()
{
    isDirty = true;
}

void DirtyFrameUpdater::OnUpdate(DAVA::float32)
{
    if (isDirty)
    {
        DVASSERT(callback, "updater is not set");
        callback();
        isDirty = false;
    }
}
