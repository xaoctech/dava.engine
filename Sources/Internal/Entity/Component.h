#pragma once

#include "Base/BaseTypes.h"
#include "Base/Serializable.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "MemoryManager/MemoryProfiler.h"
#include "Reflection/Reflection.h"

/**
    \defgroup components Component
*/

namespace DAVA
{
class Entity;

class Component : public Serializable, public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_COMPONENT)

public:
    static Component* CreateByType(const Type* componentType);
    template <typename T>
    static T* CreateByType();

    ~Component() override;

    const Type* GetType() const;
    uint32 GetRuntimeIndex() const;

    /** Clone component. Then add cloned component to specified `toEntity` if `toEntity` is not nullptr. Return cloned component. */
    virtual Component* Clone(Entity* toEntity) = 0;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline Entity* GetEntity() const;
    virtual void SetEntity(Entity* entity);

    /** This function should be implemented in each node that have data nodes inside it. */
    virtual void GetDataNodes(Set<DataNode*>& dataNodes);

    /** This function optimizes component before export. */
    virtual void OptimizeBeforeExport()
    {
    }

    /** Function to get data nodes of requested type to specific container you provide. */
    template <template <typename> class Container, class T>
    void GetDataNodes(Container<T>& container);

protected:
    Entity* entity = 0;
    mutable const Type* type = nullptr;

    DAVA_VIRTUAL_REFLECTION(Component, InspBase);
};

template <typename T>
inline T* Component::CreateByType()
{
    return DynamicTypeCheck<T*>(CreateByType(Type::Instance<T>()));
}

inline Entity* Component::GetEntity() const
{
    return entity;
};

template <template <typename> class Container, class T>
void Component::GetDataNodes(Container<T>& container)
{
    Set<DataNode*> objects;
    GetDataNodes(objects);

    Set<DataNode*>::const_iterator end = objects.end();
    for (Set<DataNode*>::iterator t = objects.begin(); t != end; ++t)
    {
        DataNode* obj = *t;

        T res = dynamic_cast<T>(obj);
        if (res != nullptr)
        {
            container.push_back(res);
        }
    }
}
};
