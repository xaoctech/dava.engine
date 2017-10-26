#pragma once

#include "ui_PackageWidget.h"

#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/SettingsNode.h>

#include <Reflection/Reflection.h>
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
class PackageModel;
class PackageNode;
class QItemSelection;
class CommandExecutor;

class PackageWidgetSettings : public DAVA::TArc::SettingsNode
{
public:
    DAVA::uint32 selectedDevice = 0;
    DAVA::uint32 selectedBlank = 0;

    bool useCustomUIViewerPath = false;
    DAVA::String customUIViewerPath;

    DAVA_VIRTUAL_REFLECTION(PackageWidgetSettings, DAVA::TArc::SettingsNode);
};

class PackageWidget : public QDockWidget, public Ui::PackageWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget* parent = 0);
    ~PackageWidget();

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);
    void SetUI(DAVA::TArc::UI* ui);
    void BindActionsToTArc();

    PackageModel* GetPackageModel() const;
    using ExpandedIndexes = QModelIndexList;

    void OnSelectionChanged(const DAVA::Any& selection);
    void OnPackageChanged(PackageContext* context, PackageNode* node);

signals:
    void SelectedNodesChanged(const SelectedNodes& selection);

public slots:
    void OnSelectionChangedFromView(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected);
    void OnRename();
    void OnBeforeProcessNodes(const SelectedNodes& nodes);
    void OnAfterProcessNodes(const SelectedNodes& nodes);

private slots:
    void ExpandToFirstChild();

private:
    void SetSelectedNodes(const SelectedNodes& selection);
    void CollectExpandedIndexes(PackageBaseNode* node);
    void LoadContext();
    void SaveContext();

    void DeselectNodeImpl(PackageBaseNode* node);
    void SelectNodeImpl(PackageBaseNode* node);

    ExpandedIndexes GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const ExpandedIndexes& indexes);

    PackageModel* packageModel = nullptr;

    SelectionContainer selectionContainer;
    SelectedNodes expandedNodes;
    //source indexes
    PackageContext* currentContext = nullptr;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
    DAVA::TArc::DataWrapper dataWrapper;
};

struct PackageContext
{
    PackageWidget::ExpandedIndexes expandedIndexes;
};
