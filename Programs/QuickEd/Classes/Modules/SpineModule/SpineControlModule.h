#pragma once

#include <memory.h>

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>
#include <Base/GlobalEnum.h>

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
    bool GetSystemPauseState() const;
    void SetSystemPauseState(bool pause);
    void RebuildAllBoneLinks();
    bool IsEnabled() const;
    void UpdateSceneSystem();

    const String pausePropertyTitle = "Pause";
    const String rebuildButtonTitle = "Rebuild all bones alinks";
};