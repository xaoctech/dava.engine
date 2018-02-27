#pragma once

#include "VisualScriptEditor/Private/VisualScriptEditorData.h"

#include <TArc/Controls/PropertyPanel/PropertiesView.h>
#include <TArc/Utils/QtDelayedExecutor.h>

#include <QDialog>

#include <memory>

namespace QtNodes
{
class Node;
}

class QToolBar;
class QTabWidget;

namespace DAVA
{
class VisualScript;
class VisualScriptFlowScene;

class VisualScriptEditorDialog : public QDialog
{
public:
    explicit VisualScriptEditorDialog(ContextAccessor* accessor, UI* ui);
    ~VisualScriptEditorDialog() override;

    void OpenScriptByPath(const QString& path);

private:
    QToolBar* CreateToolBar() const;

    void NewScript();
    void LoadScript();
    void NewScriptImpl(const FilePath& scriptPath);

    void SaveScript();

    void CompileScript();
    void ExecuteScript();

    void ClearSelection();
    void DeleteSelection();

    void StoreRecentScriptPathname(const FilePath& scriptPath);

    void OnTabChanged(int tabIndex);
    void OnTabCloseRequested(int tabIndex);

    void DeleteRemovedScripts();

    void OnNodeDoubleClicked(QtNodes::Node& n);

    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;

    QTabWidget* tabBar = nullptr;

    std::shared_ptr<PropertiesView::Updater> updater;
    PropertiesView* propertiesView = nullptr;

    Vector<ScriptDescriptor> allScripts;

    QtDelayedExecutor delayedDeleteExecutor;
    Vector<ScriptDescriptor> scriptsForDeletion;

    int32 activeScript = -1;
};

} // DAVA
