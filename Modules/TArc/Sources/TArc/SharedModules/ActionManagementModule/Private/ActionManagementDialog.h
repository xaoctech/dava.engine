#pragma once

#include "TArc/Utils/QtConnections.h"
#include <Reflection/Reflection.h>

#include <QDialog>
#include <QPointer>

class QItemSelectionModel;
class QItemSelection;
class QTreeView;

namespace DAVA
{
namespace TArc
{
class UIManager;
class ContextAccessor;
class ShortcutsModel;
class ActionManagementDialog : public QDialog
{
public:
    ActionManagementDialog(ContextAccessor* accessor, UIManager* ui);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    String GetCurrentKeyBindingsScheme() const;
    void SetCurrentKeyBindingsScheme(const String& scheme);

    void AddKeyBindingsScheme();
    void RemoveKeyBindingsScheme();
    void ImportKeyBindingsScheme();
    void ExportKeyBindingsScheme();

    void UpdateSchemes();

    void RemoveSequence();

    String GetShortcutText() const;
    void SetShortcutText(const String& text);

    bool CanBeAssigned() const;
    void AssignShortcut();

    Qt::ShortcutContext GetContext() const;
    void SetContext(Qt::ShortcutContext v);

    void OnActionSelected(const QItemSelection& selected, const QItemSelection& deselected);

private:
    ContextAccessor* accessor = nullptr;
    UIManager* ui = nullptr;
    QTreeView* treeView = nullptr;
    ShortcutsModel* shortcutsModel = nullptr;
    QtConnections connections;

    Set<String> schemes;

    QString selectedBlockName;
    QPointer<QAction> selectedAction;
    String currentSequence;
    Set<String> sequences;
    Qt::ShortcutContext context = Qt::WidgetWithChildrenShortcut;
    bool isSelectedActionReadOnly = false;

    QKeySequence shortcutText;

    DAVA_REFLECTION(ActionManagementDialog);
};
} // namespace TArc
} // namespace DAVA
