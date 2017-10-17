#include "Classes/SceneTree/Private/SceneTreeFilterModelV2.h"
#include "Classes/SceneTree/Private/SceneTreeRoles.h"

#include <Debug/DVAssert.h>

#include <QBrush>
#include <QColor>

const QString& SceneTreeFilterModelV2::GetFilter() const
{
    return filter;
}

void SceneTreeFilterModelV2::SetFilter(const QString& filter_)
{
    filter = QString("");
    filtrationData.clear();
    FullFetchModel(sourceModel(), QModelIndex());

    filter = filter_;
    if (filter.isEmpty() == false)
    {
        PrepareFiltrationData(sourceModel(), QModelIndex());
    }

    invalidate();
}

QVariant SceneTreeFilterModelV2::data(const QModelIndex& index, int role) const
{
    if (role == Qt::BackgroundRole && filter.isEmpty() == false)
    {
        QHash<QModelIndex, FilterData>::const_iterator iter = filtrationData.constFind(mapToSource(index));
        DVASSERT(iter != filtrationData.cend());
        if (iter.value().isMatched == true)
        {
            return QBrush(QColor(0, 255, 0, 20));
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

bool SceneTreeFilterModelV2::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (filter.isEmpty() == true)
    {
        return true;
    }

    QModelIndex itemIndex = sourceModel()->index(sourceRow, 0, sourceParent);

    QHash<QModelIndex, FilterData>::const_iterator iter = filtrationData.constFind(itemIndex);
    if (iter == filtrationData.cend())
    {
        PrepareFiltrationData(sourceModel(), itemIndex);
        iter = filtrationData.constFind(itemIndex);
    }
    DVASSERT(iter != filtrationData.constEnd());
    return iter.value().isVisible;
}

bool SceneTreeFilterModelV2::PrepareFiltrationData(QAbstractItemModel* model, const QModelIndex& index) const
{
    bool isSomeChildIsMatched = false;
    int rowCount = model->rowCount(index);
    for (int i = 0; i < rowCount; ++i)
    {
        isSomeChildIsMatched |= PrepareFiltrationData(model, model->index(i, 0, index));
    }

    FilterData data;
    QString displayName = index.data(Qt::DisplayRole).toString();
    QString filterItemData = index.data(ToItemRoleCast(eSceneTreeRoles::FilterDataRole)).toString();
    data.isMatched = displayName.contains(filter, Qt::CaseInsensitive) || filter == filterItemData;
    data.isVisible = isSomeChildIsMatched || data.isMatched;

    filtrationData.insert(index, data);
    return data.isVisible;
}

void SceneTreeFilterModelV2::FullFetchModel(QAbstractItemModel* model, const QModelIndex& index)
{
    if (model->canFetchMore(index) == true)
    {
        model->fetchMore(index);
    }

    int rowCount = model->rowCount(index);
    for (int i = 0; i < rowCount; ++i)
    {
        FullFetchModel(model, model->index(i, 0, index));
    }
}
