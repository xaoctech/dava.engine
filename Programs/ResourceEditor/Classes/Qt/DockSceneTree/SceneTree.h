#pragma once

#include <QWidget>
#include <QTreeView>
#include <QTableView>
#include <QTimer>
#include <QPointer>

#include "Classes/Qt/DockSceneTree/SceneTreeModel.h"
#include "Classes/Qt/DockSceneTree/SceneTreeDelegate.h"

namespace DAVA
{
class FieldBinder;
class SceneEditor2;
class RECommandNotificationObject;
}

class LazyUpdater;
class SceneTree : public QTreeView
{
    Q_OBJECT

public:
    explicit SceneTree(QWidget* parent = 0);
    ~SceneTree();

protected:
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;

private slots:
    void ShowContextMenu(const QPoint& pos);
    void SetFilter(const QString& filter);
    void RemoveSelection();

    void CollapseSwitch();
    void CollapseAll();
    void ReloadSelectedEntitiesTextures();

    void SceneActivated(DAVA::SceneEditor2* scene);
    void SceneDeactivated(DAVA::SceneEditor2* scene);
    void SceneStructureChanged(DAVA::SceneEditor2* scene, DAVA::Entity* parent);
    void CommandExecuted(DAVA::SceneEditor2* scene, const DAVA::RECommandNotificationObject& commandNotification);

    void ParticleLayerValueChanged(DAVA::SceneEditor2* scene, DAVA::ParticleLayer* layer);

    void TreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void TreeItemDoubleClicked(const QModelIndex& index);
    void TreeItemCollapsed(const QModelIndex& index);
    void TreeItemExpanded(const QModelIndex& index);

    void SyncSelectionToTree();
    void SyncSelectionFromTree();

private:
    void GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col);

    void ExpandFilteredItems();
    void BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent = QModelIndex());

    void UpdateTree();
    void UpdateModel();
    void PropagateSolidFlag();
    void PropagateSolidFlagRecursive(QStandardItem* root);

    class BaseContextMenu;
    class EntityContextMenu;
    class ParticleLayerContextMenu;
    class ParticleForceContextMenu;
    class ParticleEmitterContextMenu;
    class ParticleInnerEmitterContextMenu;

private:
    QPointer<SceneTreeModel> treeModel;
    QPointer<SceneTreeFilteringModel> filteringProxyModel;
    SceneTreeDelegate* treeDelegate = nullptr;
    LazyUpdater* treeUpdater;
    bool isInSelectionSync = false;
    std::unique_ptr<DAVA::FieldBinder> selectionFieldBinder;
};
