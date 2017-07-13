#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Reflection/Reflection.h>

class OptionWrapper;
class QDialog;

class RenderOptionsModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(RenderOptionsModule, DAVA::TArc::ClientModule);

public:
    /** Menu item name in Tools menu. */
    static const QString renderOptionsMenuItemName;

protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    void ShowRenderOptionsDialog();
    DAVA::Vector<std::shared_ptr<OptionWrapper>> optionsRefs;
    std::unique_ptr<QDialog> optionsDialog;
    DAVA::TArc::QtConnections connections;
};
