#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/QtDelayedExecutor.h>
#include <TArc/Core/OperationRegistrator.h>

#include <Reflection/Reflection.h>

namespace DAVA
{
class VisualScript;
class VisualScriptEditorDialog;
class VisualScriptEditorModule : public ClientModule
{
public:
    VisualScriptEditorModule();
    ~VisualScriptEditorModule() override;

protected:
    void PostInit() override;

private:
    void ShowEditor();
    void OpenVisualScriptFileHandler(const QString& path);

    QtConnections connections;
    QtDelayedExecutor delayedExecutor;

    VisualScriptEditorDialog* dialog = nullptr;

    DAVA_VIRTUAL_REFLECTION(VisualScriptEditorModule, ClientModule);
};

DECLARE_OPERATION_ID(OpenVisualScriptFile);
} // namespace DAVA
