#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

#include <QString>

class RenderOptionsModule : public DAVA::TArc::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(RenderOptionsModule, DAVA::TArc::ClientModule);

public:
    /** Menu item name in Tools menu. */
    static const QString renderOptionsMenuItemName;

    ~RenderOptionsModule() override;

protected:
    void PostInit() override;

private:
    void ShowRenderOptionsDialog();
    DAVA::TArc::QtConnections connections;
};
