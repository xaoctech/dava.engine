#include "Classes/SceneTree/Private/SceneTreeModelV2.h"
#include "Classes/SceneTree/Private/SceneTreeSystem.h"

#include <TArc/DataProcessing/AnyQMetaType.h>

#include <Debug/DVAssert.h>
#include <Utils/Utils.h>

SceneTreeModelV2::SceneTreeModelV2(DAVA::Scene* scene)
    : QAbstractItemModel(nullptr)
    , objectsPool(10000, 5)
{
    DVASSERT(scene != nullptr);
    rootItem = objectsPool.RequestObject();
    rootItem->object = Selectable(DAVA::Any(scene));
    mapping.emplace(rootItem->object, rootItem);

    {
        DAVA::Scene* scene = GetScene();
        DVASSERT(scene != nullptr);
        SceneTreeSystem* system = scene->GetSystem<SceneTreeSystem>();
        DVASSERT(system != nullptr);
        system->syncIsNecessary.Connect(this, &SceneTreeModelV2::OnSyncSignal);
    }
}

SceneTreeModelV2::~SceneTreeModelV2()
{
    DAVA::Scene* scene = GetScene();
    DVASSERT(scene != nullptr);
    SceneTreeSystem* system = scene->GetSystem<SceneTreeSystem>();
    DVASSERT(system != nullptr);
    system->syncIsNecessary.DisconnectAll();

    rootItem.reset();
}

int SceneTreeModelV2::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
    {
        return 0;
    }

    SceneTreeItemV2* item = MapItem(parent);
    DVASSERT(item != nullptr);
    return static_cast<int>(item->children.size());
}

int SceneTreeModelV2::columnCount(const QModelIndex& parent) const
{
    return 1;
}

bool SceneTreeModelV2::hasChildren(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
    {
        return false;
    }

    SceneTreeItemV2* item = MapItem(parent);
    DVASSERT(item != nullptr);
    return traits.GetChildrenCount(item->object) > 0;
}

QModelIndex SceneTreeModelV2::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        SceneTreeItemV2* item = MapItem(parent);
        if (item != nullptr && row < static_cast<int>(item->children.size()))
            return createIndex(row, column, item);

        return QModelIndex();
    }

    return MapItem(rootItem.get());
}

QModelIndex SceneTreeModelV2::parent(const QModelIndex& child) const
{
    DVASSERT(child.isValid());
    SceneTreeItemV2* item = reinterpret_cast<SceneTreeItemV2*>(child.internalPointer());
    if (item == nullptr)
    {
        return QModelIndex();
    }

    if (item == rootItem.get())
    {
        return MapItem(rootItem.get());
    }

    return createIndex(item->positionInParent, 0, item->parent);
}

QVariant SceneTreeModelV2::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
    {
        SceneTreeItemV2* item = MapItem(index);
        DVASSERT(item != nullptr);
        return traits.GetName(item->object);
    }
    break;
    case Qt::DecorationRole:
    {
        SceneTreeItemV2* item = MapItem(index);
        DVASSERT(item != nullptr);
        return traits.GetIcon(item->object);
    }
    break;
    case Qt::ToolTipRole:
    {
        SceneTreeItemV2* item = MapItem(index);
        DVASSERT(item != nullptr);
        return traits.GetTooltip(item->object);
    }
    break;
    case InternalObject:
    {
        SceneTreeItemV2* item = MapItem(index);
        DVASSERT(item != nullptr);
        return QVariant::fromValue(item->object.GetContainedObject());
    }
    break;
    default:
        break;
    }

    SceneTreeItemV2* item = MapItem(index);
    DVASSERT(item != nullptr);
    return traits.GetValue(item->object, role);
}

bool SceneTreeModelV2::setData(const QModelIndex& index, const QVariant& value, int role)
{
    SceneTreeItemV2* item = MapItem(index);
    return traits.SetValue(item->object, value, role, GetScene());
}

Qt::ItemFlags SceneTreeModelV2::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    SceneTreeItemV2* item = MapItem(index);
    return traits.GetItemFlags(item->object, defaultFlags);
}

QVariant SceneTreeModelV2::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section != 0)
    {
        return QVariant();
    }

    return QStringLiteral("Scene hierarchy");
}

bool SceneTreeModelV2::canFetchMore(const QModelIndex& parent) const
{
    SceneTreeItemV2* item = MapItem(parent);
    DVASSERT(item->unfetchedChildren.empty() == true);
    DAVA::int32 childCount = traits.GetChildrenCount(item->object);
    if (childCount == static_cast<DAVA::int32>(item->children.size()))
    {
        return false;
    }

    auto isFetched = [this](const Selectable& object) -> bool {
        return fetchedObjects.count(object);
    };

    traits.BuildUnfetchedList(item->object, isFetched, item->unfetchedChildren);
    return item->unfetchedChildren.empty() == false;
}

void SceneTreeModelV2::fetchMore(const QModelIndex& parent)
{
    SceneTreeItemV2* item = MapItem(parent);
    DVASSERT(item->unfetchedChildren.empty() == false);
    std::sort(item->unfetchedChildren.begin(), item->unfetchedChildren.end());

    DAVA::int32 childCount = traits.GetChildrenCount(item->object);
    traits.FetchMore(item->object, item->unfetchedChildren, [&](DAVA::int32 index, const Selectable& object) {
        beginInsertRows(parent, index, index);

        std::shared_ptr<SceneTreeItemV2> childItem = objectsPool.RequestObject();
        childItem->parent = item;
        childItem->positionInParent = index;
        childItem->object = object;

        DAVA::int32 count = static_cast<DAVA::int32>(item->children.size());
        if (index == count)
        {
            item->children.push_back(childItem);
        }
        else if (index < count)
        {
            auto insertItem = item->children.begin() + index;
            insertItem = item->children.insert(insertItem, childItem);
            for (auto iter = insertItem + 1; iter != item->children.end(); ++iter)
            {
                ++(*iter)->positionInParent;
            }
        }
        else
        {
            DVASSERT(false);
        }

        mapping.emplace(childItem->object, childItem);
        fetchedObjects.insert(childItem->object);

        endInsertRows();
    });

    item->unfetchedChildren.clear();
}

QModelIndex SceneTreeModelV2::GetRootIndex() const
{
    return MapItem(rootItem.get());
}

QModelIndex SceneTreeModelV2::GetIndexByObject(const Selectable& object) const
{
    auto iter = mapping.find(object);
    if (iter == mapping.end())
    {
        DVASSERT(false);
        return QModelIndex();
    }

    SceneTreeItemV2* item = iter->second.get();
    return MapItem(item);
}

Selectable SceneTreeModelV2::GetObjectByIndex(const QModelIndex& index) const
{
    DVASSERT(index.model() == this);
    SceneTreeItemV2* item = MapItem(index);
    DVASSERT(item != nullptr);
    return item->object;
}

void SceneTreeModelV2::OnSyncSignal()
{
    executor.DelayedExecute([this]() {
        SyncChanges();
    });
}

void SceneTreeModelV2::SyncChanges()
{
    DAVA::Scene* scene = GetScene();
    SceneTreeSystem* system = scene->GetSystem<SceneTreeSystem>();
    DVASSERT(system != nullptr);

    DAVA::Function<void(SceneTreeItemV2*)> eraseObject = [this, &eraseObject](SceneTreeItemV2* item) {
        fetchedObjects.erase(item->object);
        mapping.erase(item->object);
        for (const std::shared_ptr<SceneTreeItemV2>& child : item->children)
        {
            eraseObject(child.get());
        }
    };

    const SceneTreeSystem::SyncSnapshot& snapShot = system->GetSyncSnapshot();
    for (const auto& node : snapShot.removedObjects)
    {
        for (Selectable object : node.second)
        {
            auto iter = mapping.find(object);
            if (iter == mapping.end())
            {
                continue;
            }

            SceneTreeItemV2* item = iter->second.get();
            DVASSERT(item != nullptr);

            SceneTreeItemV2* parentItem = item->parent;
            DVASSERT(parentItem->children.empty() == false);
            QModelIndex parentIndex = MapItem(parentItem);
            beginRemoveRows(parentIndex, item->positionInParent, item->positionInParent);
            eraseObject(item);
            DAVA::int32 childrenCount = static_cast<DAVA::int32>(parentItem->children.size());
            for (DAVA::int32 i = item->positionInParent; i < parentItem->children.size() - 1; ++i)
            {
                std::swap(parentItem->children[i], parentItem->children[i + 1]);
                parentItem->children[i]->positionInParent = i;
            }
            parentItem->children.pop_back();
            endRemoveRows();
        }
    }

    for (const auto& node : snapShot.objectsToRefetch)
    {
        for (Selectable object : node.second)
        {
            auto iter = mapping.find(object);
            if (iter == mapping.end())
            {
                continue;
            }

            SceneTreeItemV2* parentItem = iter->second.get();
            QModelIndex index = MapItem(parentItem);
            if (canFetchMore(index) == true)
            {
                fetchMore(index);
            }
        }
    }

    for (const Selectable& object : snapShot.changedObjects)
    {
        auto iter = mapping.find(object);
        if (iter == mapping.end())
        {
            continue;
        }

        SceneTreeItemV2* item = iter->second.get();
        DVASSERT(item != nullptr);

        QModelIndex index = MapItem(item);
        dataChanged(index, index);
    }

    system->SyncFinished();
}

DAVA::Scene* SceneTreeModelV2::GetScene()
{
    if (rootItem == nullptr)
    {
        return nullptr;
    }

    DVASSERT(rootItem->object.CanBeCastedTo<DAVA::Entity>());
    DAVA::Entity* entity = rootItem->object.Cast<DAVA::Entity>();

    DAVA::Scene* scene = dynamic_cast<DAVA::Scene*>(entity);
    DVASSERT(scene != nullptr);
    return scene;
}

SceneTreeItemV2* SceneTreeModelV2::MapItem(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return nullptr;
    }

    SceneTreeItemV2* p = reinterpret_cast<SceneTreeItemV2*>(index.internalPointer());
    if (p == nullptr)
    {
        return rootItem.get();
    }
    return p->children[index.row()].get();
}

QModelIndex SceneTreeModelV2::MapItem(SceneTreeItemV2* item) const
{
    DVASSERT(item != nullptr);
    if (item == rootItem.get())
    {
        return createIndex(0, 0, nullptr);
    }

    return createIndex(item->positionInParent, 0, item->parent);
}