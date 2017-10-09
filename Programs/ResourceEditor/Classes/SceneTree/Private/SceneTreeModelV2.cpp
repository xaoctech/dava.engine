#include "Classes/SceneTree/Private/SceneTreeModelV2.h"
#include "Classes/SceneTree/Private/SceneTreeSystem.h"

#include <Debug/DVAssert.h>
#include "Utils/Utils.h"

SceneTreeModelV2::SceneTreeModelV2(DAVA::Scene* scene)
    : QAbstractItemModel(nullptr)
    , objectsPool(10000, 5)
{
    DVASSERT(scene != nullptr);
    rootItem = objectsPool.RequestObject();
    rootItem->entity = scene;
    mapping.emplace(rootItem->entity, rootItem);

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
    DVASSERT(item->entity != nullptr);
    return item->entity->GetChildrenCount() > 0;
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
    if (role == Qt::DisplayRole)
    {
        SceneTreeItemV2* item = MapItem(index);
        DVASSERT(item != nullptr);
        DAVA::Entity* entity = item->entity;
        DVASSERT(entity != nullptr);
        DAVA::FastName name = entity->GetName();
        DVASSERT(name.IsValid());
        return QString(name.c_str());
    }

    return QVariant();
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
    DVASSERT(item->entity != nullptr);
    DAVA::Entity* entity = item->entity;
    DAVA::int32 childCount = entity->GetChildrenCount();
    if (childCount == static_cast<DAVA::int32>(item->children.size()))
    {
        return false;
    }

    for (DAVA::int32 childIndex = 0; childIndex < childCount; ++childIndex)
    {
        DAVA::Entity* childEntity = entity->GetChild(childIndex);
        if (std::binary_search(fetchedEntities.begin(), fetchedEntities.end(), childEntity) == false)
        {
            item->unfetchedChildren.push_back(childIndex);
        }
    }
    return item->unfetchedChildren.empty() == false;
}

void SceneTreeModelV2::fetchMore(const QModelIndex& parent)
{
    SceneTreeItemV2* item = MapItem(parent);
    DVASSERT(item->entity != nullptr);
    DVASSERT(item->unfetchedChildren.empty() == false);
    std::sort(item->unfetchedChildren.begin(), item->unfetchedChildren.end());

    DAVA::int32 childCount = item->entity->GetChildrenCount();
    DVASSERT(childCount > 0);
    item->children.reserve(childCount);
    for (DAVA::int32 index : item->unfetchedChildren)
    {
        DVASSERT(index < childCount);

        beginInsertRows(parent, index, index);
        DAVA::Entity* childEntity = item->entity->GetChild(index);

        std::shared_ptr<SceneTreeItemV2> childItem = objectsPool.RequestObject();
        childItem->parent = item;
        childItem->positionInParent = static_cast<DAVA::int32>(item->children.size());
        childItem->entity = childEntity;

        DAVA::int32 count = static_cast<DAVA::int32>(item->children.size());
        if (index == count)
        {
            item->children.push_back(childItem);
        }
        else if (index < count)
        {
            auto insertItem = item->children.begin() + index;
            insertItem = item->children.insert(insertItem, childItem);
            for (auto iter = insertItem; iter != insertItem; ++iter)
            {
                ++(*iter)->positionInParent;
            }
        }
        else
        {
            DVASSERT(false);
        }

        mapping.emplace(childItem->entity, childItem);
        fetchedEntities.push_back(childEntity);
        endInsertRows();
    }

    item->unfetchedChildren.clear();
    std::sort(fetchedEntities.begin(), fetchedEntities.end());
}

QModelIndex SceneTreeModelV2::GetRootIndex() const
{
    return MapItem(rootItem.get());
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

    const SceneTreeSystem::SyncSnapshot& snapShot = system->GetSyncSnapshot();
    for (const auto& node : snapShot.removedEntities)
    {
        for (DAVA::Entity* entity : node.second)
        {
            auto iter = mapping.find(entity);
            DVASSERT(iter != mapping.end());
            SceneTreeItemV2* item = iter->second.get();
            DVASSERT(item != nullptr);

            SceneTreeItemV2* parentItem = item->parent;
            DVASSERT(parentItem->children.empty() == false);
            QModelIndex parentIndex = MapItem(parentItem);
            beginRemoveRows(parentIndex, item->positionInParent, item->positionInParent);
            DAVA::FindAndRemoveExchangingWithLast(fetchedEntities, item->entity);
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

    DAVA::Set<DAVA::Entity*> refetchedParents;
    for (const auto& node : snapShot.addedEntities)
    {
        for (DAVA::Entity* entity : node.second)
        {
            DAVA::Entity* parent = entity->GetParent();
            DVASSERT(parent != nullptr);
            if (refetchedParents.count(parent) > 0)
            {
                continue;
            }
            refetchedParents.insert(parent);

            auto iter = mapping.find(parent);
            DVASSERT(iter != mapping.end());
            SceneTreeItemV2* parentItem = iter->second.get();

            QModelIndex index = MapItem(parentItem);
            if (canFetchMore(index) == true)
            {
                fetchMore(index);
            }
        }
    }
    std::sort(fetchedEntities.begin(), fetchedEntities.end());
    system->SyncFinished();
}

DAVA::Scene* SceneTreeModelV2::GetScene()
{
    if (rootItem == nullptr)
    {
        return nullptr;
    }

    DAVA::Scene* scene = dynamic_cast<DAVA::Scene*>(rootItem->entity);
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