#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "FileSystem/KeyedArchive.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "Render/RenderHelper.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Private/EntityHelpers.h"
#include "Utils/Random.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Reflection/ReflectionRegistrator.h"
#include <functional>

namespace DAVA
{

// Property Names
const char* Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME = "editor.isSolid";
const char* Entity::SCENE_NODE_IS_LOCKED_PROPERTY_NAME = "editor.isLocked";
const char* Entity::SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME = "editor.isNotRemovable";

DAVA_VIRTUAL_REFLECTION_IMPL(Entity)
{
    ReflectionRegistrator<Entity>::Begin()
    .DestructorByPointer([](Entity* e) { DAVA::SafeRelease(e); })
    .Field("ID", &Entity::GetID, &Entity::SetID)[M::ReadOnly()]
    .Field("Name", &Entity::GetName, static_cast<void (Entity::*)(const FastName&)>(&Entity::SetName))
    .Field("Flags", &Entity::flags)[M::FlagsT<Entity::EntityFlags>()]
    .Field("Visible", &Entity::GetVisible, &Entity::SetVisible)[M::ValueDescription(&VisibleValueDescription)]
    .Field(componentFieldString, &Entity::components)
    .End();
}

const char* Entity::componentFieldString = "Components";

Entity::Entity()
{
    UpdateFamily();

    AddComponent(new TransformComponent());
}

Entity::~Entity()
{
    RemoveAllChildren();
    RemoveAllComponents();
    SetScene(nullptr);
    EntityFamily::Release(family);
}

bool ComponentLessPredicate(Component* left, Component* right)
{
    return left->GetType() < right->GetType();
}

void Entity::AddComponent(Component* component)
{
    component->SetEntity(this);
    components.push_back(component);

    std::stable_sort(components.begin(), components.end(), ComponentLessPredicate);
    UpdateFamily();

    if (scene)
        scene->RegisterComponent(this, component);
}

void Entity::DetachComponent(Vector<Component*>::iterator& it)
{
    Component* c = *it;

    if (scene)
    {
        scene->UnregisterComponent(this, c);
    }

    components.erase(it);
    UpdateFamily();
    c->SetEntity(nullptr);
}

Component* Entity::GetComponent(uint32 componentType, uint32 index) const
{
    Component* ret = nullptr;
    uint32 maxCount = family->GetComponentsCount(componentType);
    if (index < maxCount)
    {
        ret = components[family->GetComponentIndex(componentType, index)];
    }

    return ret;
}

Component* Entity::GetOrCreateComponent(uint32 componentType, uint32 index)
{
    Component* ret = GetComponent(componentType, index);
    if (!ret)
    {
        ret = Component::CreateByType(componentType);
        AddComponent(ret);
    }

    return ret;
}

void Entity::SetScene(Scene* _scene)
{
    if (scene == _scene)
    {
        return;
    }

    if (scene)
    {
        scene->UnregisterEntity(this);
    }

    scene = _scene;

    if (scene)
    {
        scene->RegisterEntity(this);
        for (auto component : components)
        {
            GlobalEventSystem::Instance()->PerformAllEventsFromCache(component);
        }
    }

    for (auto child : children)
    {
        child->SetScene(scene);
    }
}

void Entity::SetParent(Entity* _parent)
{
    parent = _parent;
    static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT))->SetParent(parent);
}

void Entity::AddNode(Entity* node)
{
    if (node)
    {
        node->Retain();
        if (node->parent)
        {
            node->parent->RemoveNode(node);
        }
        uint32 insertPosition = static_cast<uint32>(children.size());
        children.push_back(node);
        node->SetParent(this);
        node->SetScene(GetScene());
    }
}

void Entity::InsertBeforeNode(Entity* newNode, Entity* beforeNode)
{
    if (newNode && newNode != beforeNode)
    {
        bool canBeInserted = false;

        //need 2 passes because iterator will be invalidated when the entity
        //is already present in the list of children and changes its position.

        Vector<Entity*>::iterator itEnd = children.end();
        for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
        {
            if (beforeNode == (*it))
            {
                canBeInserted = true;
                break;
            }
        }

        if (canBeInserted)
        {
            newNode->Retain();
            if (newNode->parent)
            {
                newNode->parent->RemoveNode(newNode);
            }

            itEnd = children.end();
            for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
            {
                if (beforeNode == (*it))
                {
                    children.insert(it, newNode);
                    newNode->SetParent(this);
                    newNode->SetScene(GetScene());
                    break;
                }
            }
        }
    }
}

void Entity::RemoveNode(Entity* node)
{
    if (!node)
    {
        return;
    }

    const Vector<Entity*>::iterator& childrenEnd = children.end();
    for (Vector<Entity*>::iterator t = children.begin(); t != childrenEnd; ++t)
    {
        if (*t == node)
        {
            children.erase(t);
            if (node)
            {
                node->SetScene(nullptr);
                node->SetParent(nullptr);
                node->Release();
            }
            break;
        }
    }
}

Entity* Entity::GetNextChild(Entity* child)
{
    Entity* next = nullptr;

    for (uint32 i = 0; i < children.size(); i++)
    {
        if (children[i] == child)
        {
            if ((i + 1) < children.size())
            {
                next = children[i + 1];
            }
            break;
        }
    }

    return next;
}

void Entity::RemoveAllChildren()
{
    for (Vector<Entity*>::iterator t = children.begin(); t != children.end(); ++t)
    {
        Entity* node = *t;
        node->SetScene(nullptr);
        node->SetParent(nullptr);
        node->Release();
    }
    children.clear();
}

Entity* Entity::FindByName(const FastName& searchName)
{
    if (name == searchName)
        return this;

    for (auto child : children)
    {
        Entity* res = child->FindByName(searchName);
        if (res != 0)
        {
            return res;
        }
    }
    return 0;
}

Entity* Entity::FindByName(const char* searchName)
{
    return FindByName(FastName(searchName));
}

void Entity::BakeTransforms()
{
    size_t size = children.size();
    if (size == 1 && (0 == GetComponent(Component::RENDER_COMPONENT) && 0 == GetComponent(Component::PARTICLE_EFFECT_COMPONENT) && 0 == GetComponent(Component::ANIMATION_COMPONENT))) // propagate matrices
    {
        children[0]->SetLocalTransform(children[0]->GetLocalTransform() * GetLocalTransform());
        SetLocalTransform(Matrix4::IDENTITY);
    }

    for (auto child : children)
    {
        child->BakeTransforms();
    }
}

void Entity::SceneDidLoaded()
{
    for (auto child : children)
        child->SceneDidLoaded();
}

Entity* Entity::Clone(Entity* dstNode)
{
    if (!dstNode)
    {
        DVASSERT(IsPointerToExactClass<Entity>(this), "Can clone only Entity");
        dstNode = new Entity();
    }

    dstNode->RemoveAllComponents();
    for (auto component : components)
    {
        dstNode->AddComponent(component->Clone(dstNode));
    }

    dstNode->name = name;
    dstNode->sceneId = sceneId;
    dstNode->id = 0;

    //flags are intentionally not cloned
    //dstNode->flags = flags;

    dstNode->RemoveAllChildren();
    dstNode->children.reserve(children.size());

    for (auto child : children)
    {
        Entity* n = child->Clone();
        dstNode->AddNode(n);
        n->Release();
    }

    return dstNode;
}

void Entity::SetDebugFlags(uint32 debugFlags, bool isRecursive)
{
    DebugRenderComponent* debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));

    if (!debugComponent)
    {
        AddComponent(new DebugRenderComponent());
        debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));
        debugComponent->SetDebugFlags(DebugRenderComponent::DEBUG_AUTOCREATED);
    }

    debugComponent->SetDebugFlags(debugFlags);
    if (0 == debugFlags)
    {
        if (debugComponent->GetDebugFlags() & DebugRenderComponent::DEBUG_AUTOCREATED)
        {
            RemoveComponent(Component::DEBUG_RENDER_COMPONENT);
        }
    }

    if (isRecursive)
    {
        for (auto child : children)
        {
            child->SetDebugFlags(debugFlags, isRecursive);
        }
    }
}

uint32 Entity::GetDebugFlags() const
{
    DebugRenderComponent* debugComponent = cast_if_equal<DebugRenderComponent*>(GetComponent(Component::DEBUG_RENDER_COMPONENT));
    if (debugComponent)
    {
        return debugComponent->GetDebugFlags();
    }

    return 0;
}

void Entity::SetName(const FastName& _name)
{
    name = _name;
}

void Entity::SetName(const char* _name)
{
    name = FastName(_name);
}

String Entity::GetFullName()
{
    return RecursiveBuildFullName(this, scene);
}

String Entity::RecursiveBuildFullName(Entity* node, Entity* endNode)
{
    if (!node)
        return "";

    if (node->GetParent() != endNode)
    {
        return RecursiveBuildFullName(node->GetParent(), endNode) + String("->") + String(node->name.c_str());
    }
    else
    {
        return String(node->name.c_str());
    }
}

bool Entity::FindNodesByNamePart(const String& namePart, List<Entity*>& outNodeList)
{
    bool isFind = false;
    size_t fp = name.find(namePart);
    if (fp != String::npos)
    {
        outNodeList.push_back(this);
        isFind = true;
    }

    for (auto child : children)
    {
        if (child->FindNodesByNamePart(namePart, outNodeList))
        {
            isFind = true;
        }
    }

    return isFind;
}

AABBox3 Entity::GetWTMaximumBoundingBoxSlow()
{
    AABBox3 retBBox;

    RenderComponent* renderComponent = static_cast<RenderComponent*>(GetComponent(Component::RENDER_COMPONENT));
    TransformComponent* transformComponent = static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT));
    if (renderComponent && transformComponent)
    {
        AABBox3 wtBox;
        renderComponent->GetRenderObject()->GetBoundingBox().GetTransformedBox(transformComponent->GetWorldTransform(), wtBox);
        retBBox.AddAABBox(wtBox);
    }

    for (auto child : children)
    {
        AABBox3 box = child->GetWTMaximumBoundingBoxSlow();
        if ((AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
            && (-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
        {
            retBBox.AddAABBox(box);
        }
    }

    return retBBox;
}

void Entity::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::SaveObject(archive);

    archive->SetString("name", String(name.c_str()));
    archive->SetUInt32("id", id);
    archive->SetUInt32("flags", flags);

    KeyedArchive* compsArch = new KeyedArchive();
    uint32 savedIndex = 0;
    for (Component* c : components)
    {
        if (c->GetType() < Component::DEBUG_COMPONENTS)
        {
            //don't save empty custom properties
            if (c->GetType() == Component::CUSTOM_PROPERTIES_COMPONENT)
            {
                CustomPropertiesComponent* customProps = cast_if_equal<CustomPropertiesComponent*>(c);
                if (customProps && customProps->GetArchive()->Count() <= 0)
                {
                    continue;
                }
            }

            KeyedArchive* compArch = new KeyedArchive();
            c->Serialize(compArch, serializationContext);
            compsArch->SetArchive(KeyedArchive::GenKeyFromIndex(savedIndex), compArch);
            compArch->Release();
            savedIndex++;
        }
    }

    compsArch->SetUInt32("count", savedIndex);
    archive->SetArchive("components", compsArch);
    compsArch->Release();
}

void Entity::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::LoadObject(archive);

    name = FastName(archive->GetString("name", "").c_str());
    id = archive->GetUInt32("id", 0);
    if (nullptr != serializationContext->GetScene())
    {
        sceneId = serializationContext->GetScene()->GetSceneID();
    }

    flags = archive->GetUInt32("flags", NODE_VISIBLE);
    flags &= ~TRANSFORM_DIRTY;

    KeyedArchive* compsArch = archive->GetArchive("components");

    LoadComponentsV7(compsArch, serializationContext);
}

void Entity::LoadComponentsV7(KeyedArchive* compsArch, SerializationContext* serializationContext)
{
    if (nullptr != compsArch)
    {
        uint32 componentCount = compsArch->GetUInt32("count");
        for (uint32 i = 0; i < componentCount; ++i)
        {
            KeyedArchive* compArch = compsArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (nullptr != compArch)
            {
                String componentType = compArch->GetString("comp.typename");
                Component* comp = ObjectFactory::Instance()->New<Component>(componentType);
                if (nullptr != comp)
                {
                    if (comp->GetType() == Component::TRANSFORM_COMPONENT)
                    {
                        RemoveComponent(comp->GetType());
                    }

                    AddComponent(comp);
                    comp->Deserialize(compArch, serializationContext);
                }
            }
        }
    }
}

void Entity::SetSolid(bool isSolid)
{
    KeyedArchive* props = GetOrCreateCustomProperties(this)->GetArchive();
    props->SetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, isSolid);
}

bool Entity::GetSolid() const
{
    KeyedArchive* props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
    }
    return false;
}

void Entity::SetLocked(bool isLocked)
{
    KeyedArchive* props = GetOrCreateCustomProperties(this)->GetArchive();
    props->SetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, isLocked);
}

bool Entity::GetLocked() const
{
    KeyedArchive* props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_LOCKED_PROPERTY_NAME, false);
    }
    return false;
}

void Entity::SetNotRemovable(bool notRemovable)
{
    KeyedArchive* props = GetOrCreateCustomProperties(this)->GetArchive();
    props->SetBool(SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME, notRemovable);
}

bool Entity::GetNotRemovable() const
{
    KeyedArchive* props = GetCustomPropertiesArchieve(this);
    if (nullptr != props)
    {
        return props->GetBool(SCENE_NODE_IS_NOT_REMOVABLE_PROPERTY_NAME, false);
    }
    return false;
}

void Entity::GetDataNodes(Set<DataNode*>& dataNodes)
{
    size_t size = components.size();
    for (size_t k = 0; k < size; ++k)
    {
        components[k]->GetDataNodes(dataNodes);
    }

    size = children.size();
    for (size_t c = 0; c < size; ++c)
    {
        children[c]->GetDataNodes(dataNodes);
    }
}

void Entity::OptimizeBeforeExport()
{
    for (uint32 i = 0; i < Component::COMPONENT_COUNT; ++i)
    {
        uint32 componentsCount = GetComponentCount(i);
        for (uint32 index = 0; index < componentsCount; ++index)
        {
            Component* c = GetComponent(i, index);
            c->OptimizeBeforeExport();
        }
    }

    size_t size = children.size();
    for (size_t c = 0; c < size; ++c)
    {
        children[c]->OptimizeBeforeExport();
    }
}

void Entity::AddFlagRecursive(int32 flagToAdd)
{
    AddFlag(flagToAdd);
    const Vector<Entity*>::iterator& itEnd = children.end();
    for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        (*it)->AddFlagRecursive(flagToAdd);
    }
}

void Entity::RemoveFlagRecursive(int32 flagToRemove)
{
    RemoveFlag(flagToRemove);
    const Vector<Entity*>::iterator& itEnd = children.end();
    for (Vector<Entity*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        (*it)->RemoveFlagRecursive(flagToRemove);
    }
}

void Entity::SetLocalTransform(const Matrix4& newMatrix)
{
    //	TIME_PROFILE("Entity::SetLocalTransform");
    (static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT)))->SetLocalTransform(&newMatrix);
}

const Matrix4& Entity::GetLocalTransform()
{
    return (static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT)))->GetLocalTransform();
}

Matrix4 Entity::AccamulateLocalTransform(Entity* fromParent)
{
    if (fromParent == this)
    {
        return GetLocalTransform();
    }
    return GetLocalTransform() * parent->AccamulateLocalTransform(fromParent);
}

Matrix4 Entity::AccamulateTransformUptoFarParent(Entity* farParent)
{
    if (farParent == this)
    {
        return Matrix4::IDENTITY;
    }
    return GetLocalTransform() * parent->AccamulateTransformUptoFarParent(farParent);
}

void Entity::FindComponentsByTypeRecursive(Component::eType type, List<DAVA::Entity*>& components)
{
    Component* component = GetComponent(type);
    if (nullptr != component)
    {
        components.push_back(this);
    }

    uint32 childCount = GetChildrenCount();
    for (uint32 i = 0; i < childCount; ++i)
    {
        GetChild(i)->FindComponentsByTypeRecursive(type, components);
    }
}

uint32 Entity::CountChildEntitiesWithComponent(Component::eType type, bool recursive /* = false */) const
{
    uint32 count = 0;
    for (auto childEntity : children)
    {
        if (childEntity->GetComponent(type))
        {
            ++count;
        }
        if (recursive)
        {
            count += childEntity->CountChildEntitiesWithComponent(type, recursive);
        }
    }
    return count;
}

inline void Entity::RemoveComponent(Vector<Component*>::iterator& it)
{
    if (it != components.end())
    {
        Component* c = *it;
        DetachComponent(it);
        SafeDelete(c);
    }
}

void Entity::SetVisible(const bool& isVisible)
{
    RenderComponent* renderComponent = static_cast<RenderComponent*>(GetComponent(Component::RENDER_COMPONENT));
    if (isVisible)
    {
        AddFlag(NODE_VISIBLE);

        if (nullptr != renderComponent)
        {
            RenderObject* renderObject = renderComponent->GetRenderObject();
            renderObject->SetFlags(renderObject->GetFlags() | RenderObject::VISIBLE);
            if ((renderObject->GetFlags() & RenderObject::NEED_UPDATE) &&
                renderObject->GetRenderSystem() != nullptr)
            {
                renderObject->GetRenderSystem()->MarkForUpdate(renderObject);
            }
        }
    }
    else
    {
        RemoveFlag(NODE_VISIBLE);
        if (nullptr != renderComponent)
        {
            RenderObject* renderObject = renderComponent->GetRenderObject();
            renderObject->SetFlags(renderObject->GetFlags() & ~RenderObject::VISIBLE);
        }
    }

    ParticleEffectComponent* effect = GetEffectComponent(this);
    if (nullptr != effect)
        effect->SetRenderObjectVisible(isVisible);

    int32 count = GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        GetChild(i)->SetVisible(isVisible);
    }
}

const Matrix4& Entity::GetWorldTransform() const
{
    return (static_cast<TransformComponent*>(GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransform();
}

Entity* Entity::GetEntityByID(uint32 id)
{
    Entity* ret = nullptr;

    if (this->id == id)
    {
        ret = this;
    }
    else
    {
        for (auto child : children)
        {
            ret = child->GetEntityByID(id);
            if (nullptr != ret)
            {
                break;
            }
        }
    }

    return ret;
}
};
