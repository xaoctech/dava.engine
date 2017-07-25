#pragma once

#include "ui_PackageWidget.h"

#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Preferences/PreferencesRegistrator.h>
#include <Base/BaseTypes.h>

#include <QWidget>
#include <QDockWidget>
#include <QModelIndex>
#include <QStack>
#include <QPointer>

#include <memory>

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
class UI;
}
}

struct PackageContext;
class ControlNode;
class StyleSheetNode;
class PackageBaseNode;
class FilteredPackageModel;
class PackageModel;
class PackageNode;
class QItemSelection;
class CommandExecutor;

class PackageWidget : public QDockWidget, public Ui::PackageWidget, public DAVA::InspBase
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget* parent = 0);
    ~PackageWidget();

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);
    void SetUI(DAVA::TArc::UI* ui);

    PackageModel* GetPackageModel() const;
    using ExpandedIndexes = QModelIndexList;

    void OnSelectionChanged(const DAVA::Any& selection);
    void OnPackageChanged(PackageContext* context, PackageNode* node);

signals:
    void SelectedNodesChanged(const SelectedNodes& selection);

public slots:
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();
    void OnDuplicate();
    void OnImport();

    void OnSelectionChangedFromView(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected);
    void OnFilterTextChanged(const QString&);
    void OnRename();
    void OnAddStyle();
    void OnCopyControlPath();
    void OnMoveUp();
    void OnMoveDown();
    void OnMoveLeft();
    void OnMoveRight();
    void OnBeforeProcessNodes(const SelectedNodes& nodes);
    void OnAfterProcessNodes(const SelectedNodes& nodes);

    void OnRunUIViewer();
    void OnRunUIViewerFast();

private:
    void PushErrorMessage(const DAVA::String& errorMessage);

    void SetSelectedNodes(const SelectedNodes& selection);
    void CollectExpandedIndexes(PackageBaseNode* node);
    void MoveNodeUpDown(bool up);
    void MoveNodeImpl(PackageBaseNode* node, PackageBaseNode* dest, DAVA::uint32 destIndex);
    QAction* CreateAction(const QString& name, void (PackageWidget::*callback)(void), const QKeySequence& sequence = QKeySequence());
    void CreateActions();
    void PlaceActions();
    void RefreshActions();
    void LoadContext();
    void SaveContext();
    void Paste(PackageBaseNode* target, int index);

    void DeselectNodeImpl(PackageBaseNode* node);
    void SelectNodeImpl(PackageBaseNode* node);
    void CollectSelectedControls(DAVA::Vector<ControlNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedStyles(DAVA::Vector<StyleSheetNode*>& nodes, bool forCopy, bool forRemove);
    void CollectSelectedImportedPackages(DAVA::Vector<PackageNode*>& nodes, bool forCopy, bool forRemove);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*>& controls, const DAVA::Vector<StyleSheetNode*>& styles);

    ExpandedIndexes GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const ExpandedIndexes& indexes);

    QAction* importPackageAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;
    QAction* delAction = nullptr;
    QAction* duplicateControlsAction = nullptr;

    QAction* renameAction = nullptr;
    QAction* addStyleAction = nullptr;
    QAction* copyControlPathAction = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    QAction* moveLeftAction = nullptr;
    QAction* moveRightAction = nullptr;

    QAction* runUIViewerFast = nullptr;
    QAction* runUIViewer = nullptr;

    FilteredPackageModel* filteredPackageModel = nullptr;
    PackageModel* packageModel = nullptr;

    SelectionContainer selectionContainer;
    SelectedNodes expandedNodes;
    //source indexes
    bool lastFilterTextEmpty = true;
    PackageContext* currentContext = nullptr;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
    DAVA::TArc::DataWrapper dataWrapper;

    DAVA::uint32 selectedDevice = 0;
    DAVA::uint32 selectedBlank = 0;

    bool useCustomUIViewerPath = false;
    DAVA::String customUIViewerPath;

public:
    INTROSPECTION(PackageWidget,
                  MEMBER(selectedDevice, "Selected Device Index", DAVA::I_SAVE | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  MEMBER(selectedBlank, "Selected Blank Index", DAVA::I_SAVE | DAVA::I_EDIT | DAVA::I_PREFERENCE)

                  MEMBER(useCustomUIViewerPath, "Package Widget/Override UIViewer Path", DAVA::I_SAVE | DAVA::I_EDIT | DAVA::I_VIEW | DAVA::I_PREFERENCE)
                  MEMBER(customUIViewerPath, "Package Widget/UIViewer path", DAVA::I_SAVE | DAVA::I_EDIT | DAVA::I_VIEW | DAVA::I_PREFERENCE)
                  )
};

struct PackageContext
{
    PackageWidget::ExpandedIndexes expandedIndexes;
    QString filterString;
};
