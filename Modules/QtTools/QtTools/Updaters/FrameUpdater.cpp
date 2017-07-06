#include "QtTools/Updaters/FrameUpdater.h"

#include <Engine/Engine.h>

FrameUpdater::FrameUpdater()
{
    DAVA::Engine::Instance()->update.Connect(this, &FrameUpdater::OnUpdate);
}

void FrameUpdater::SetUpdater(const Updater& updater_)
{
    updater = updater_;
}

void FrameUpdater::Update()
{
    needUpdate = true;
}

void FrameUpdater::OnUpdate(DAVA::float32)
{
    if (needUpdate)
    {
        DVASSERT(updater, "updater is not set");
        updater();
        needUpdate = false;
    }
}
