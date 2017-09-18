#pragma once

#include <TArc/Core/ClientModule.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

class QIcon;

namespace DAVA
{
class UISpineSystem;
}

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
    DAVA::UISpineSystem* GetSpineSystem() const;

    const DAVA::String pauseButtonHint = "Play/Pause Spine animations";
    const DAVA::String rebuildButtonHint = "Rebuild all links Spine bones to controls";
};