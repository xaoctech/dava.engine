#pragma once

#include <TArc/Core/ClientModule.h>

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Reflection/Reflection.h>

#include <QIcon>

class OptionWrapper;
class QDialog;

class RenderOptionsModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(RenderOptionsModule, DAVA::TArc::ClientModule);

protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    bool IsEnabled() const;
    const QIcon& GetToolbarButtonIcon() const;
    const DAVA::String& GetToolbarButtonHint() const;
    void ShowRenderOptionsDialog();
    Vector<std::shared_ptr<OptionWrapper>> optionsRefs;
    std::unique_ptr<QDialog> optionsDialog;
};
