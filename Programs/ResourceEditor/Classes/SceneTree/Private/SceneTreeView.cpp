#include "Classes/SceneTree/Private/SceneTreeView.h"
#include "Classes/SceneTree/Private/SceneTreeModelV2.h"

#include <Debug/DVAssert.h>

#include <QItemSelectionModel>

SceneTreeView::SceneTreeView(const Params& params, DAVA::TArc::ContextAccessor* accessor, DAVA::Reflection model, QWidget* parent)
    : ControlProxyImpl<QTreeView>(params, DAVA::TArc::ControlDescriptor(params.fields), accessor, model, parent)
    , defaultSelectionModel(new QItemSelectionModel(nullptr, this))
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setExpandsOnDoubleClick(false);
    setUniformRowHeights(true);

    connections.AddConnection(this, &QTreeView::expanded, DAVA::MakeFunction(this, &SceneTreeView::OnItemExpanded));
    connections.AddConnection(this, &QTreeView::collapsed, DAVA::MakeFunction(this, &SceneTreeView::OnItemCollapsed));
}

void SceneTreeView::UpdateControl(const DAVA::TArc::ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::DataModel) == true)
    {
        SceneTreeModelV2* model = GetFieldValue<SceneTreeModelV2*>(Fields::DataModel, nullptr);
        QModelIndex rootIndex;
        if (model != nullptr)
        {
            rootIndex = model->GetRootIndex();
        }

        setModel(model);
        setRootIndex(rootIndex);
    }

    if (descriptor.IsChanged(Fields::ExpandedIndexList) == true)
    {
        expandedIndexList = GetFieldValue(Fields::ExpandedIndexList, DAVA::Set<QPersistentModelIndex>());
        collapseAll();
        for (const QPersistentModelIndex& index : expandedIndexList)
        {
            if (index.isValid() == true)
            {
                DVASSERT(static_cast<QAbstractItemView*>(this)->model() == index.model());
                expand(index);
            }
        }
    }

    if (descriptor.IsChanged(Fields::SelectionModel) == true)
    {
        QItemSelectionModel* selectionModel = GetFieldValue<QItemSelectionModel*>(Fields::SelectionModel, nullptr);
        if (selectionModel != nullptr)
        {
            setSelectionModel(selectionModel);
        }
    }
}

void SceneTreeView::OnItemExpanded(const QModelIndex& index)
{
    expandedIndexList.insert(QPersistentModelIndex(index));
    wrapper.SetFieldValue(GetFieldName(Fields::ExpandedIndexList), expandedIndexList);
}

void SceneTreeView::OnItemCollapsed(const QModelIndex& index)
{
    expandedIndexList.erase(QPersistentModelIndex(index));
    wrapper.SetFieldValue(GetFieldName(Fields::ExpandedIndexList), expandedIndexList);
}
