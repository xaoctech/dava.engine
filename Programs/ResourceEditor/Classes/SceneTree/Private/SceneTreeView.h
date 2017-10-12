#pragma once

#include <TArc/Controls/ControlProxy.h>
#include <TArc/Controls/ControlDescriptor.h>
#include <TArc/Utils/QtConnections.h>

#include <QTreeView>

class QItemSelectionModel;
namespace DAVA
{
namespace TArc
{
class ContextAccessor;
} // namespace TArc
} // namespace DAVA

class SceneTreeView : public DAVA::TArc::ControlProxyImpl<QTreeView>
{
public:
    enum class Fields
    {
        DataModel,
        SelectionModel,
        ExpandedIndexList,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    SceneTreeView(const Params& params, DAVA::TArc::ContextAccessor* accessor, DAVA::Reflection model, QWidget* parent = nullptr);

protected:
    void UpdateControl(const DAVA::TArc::ControlDescriptor& descriptor) override;

    void OnItemExpanded(const QModelIndex& index);
    void OnItemCollapsed(const QModelIndex& index);

private:
    DAVA::TArc::QtConnections connections;
    DAVA::Set<QPersistentModelIndex> expandedIndexList;
    QItemSelectionModel* defaultSelectionModel = nullptr;
};
