#pragma once

#include <memory.h>

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>
#include <Base/GlobalEnum.h>

#include <QIcon>

namespace DAVA
{
namespace TArc
{
class FieldBinder;
}
}

class SelectableGroup;

class SpineControlModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(SpineControlModule, DAVA::TArc::ClientModule);

protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    bool IsEnabled() const;
    const QIcon& GetRebuildButtonIcon();
    const QIcon& GetPauseButtonIcon();
    void RebuildAllBoneLinks();
    void PlayPause();

    const String pauseButtonHint = "Play/Pause Spine animations";
    const String rebuildButtonHint = "Rebuild all links Spine bones to controls";
};