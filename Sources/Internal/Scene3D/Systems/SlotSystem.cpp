#include "Scene3D/Systems/SlotSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/SlotComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Systems/Private/AsyncSlotExternalLoader.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/XMLParser.h"
#include "FileSystem/YamlNode.h"
#include "Logger/Logger.h"

namespace DAVA
{
class SlotSystem::ItemsCache::XmlConfigParser : public XMLParserDelegate
{
public:
    XmlConfigParser(Set<SlotSystem::ItemsCache::Item, SlotSystem::ItemsCache::ItemLess>* items);

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override;
    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override;
    void OnFoundCharacters(const String& chars) override;

    bool IsErrorFound() const;
    bool IsDublicatesFound() const;

private:
    Set<Item, ItemLess>* items;
    bool errorFound = false;
    bool duplicatesFound = false;
};

SlotSystem::ItemsCache::XmlConfigParser::XmlConfigParser(Set<Item, ItemLess>* items_)
    : items(items_)
{
}

void SlotSystem::ItemsCache::XmlConfigParser::OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes)
{
    String nameKey("Name");
    String typeKey("Type");
    String pathKey("Path");

    if (elementName == "item")
    {
        Item item;
        for (const auto& attributeNode : attributes)
        {
            const String& key = attributeNode.first;
            const String& value = attributeNode.second;
            if (key == nameKey)
            {
                item.itemName = FastName(value);
            }
            else if (key == typeKey)
            {
                item.type = FastName(value);
            }
            else if (key == pathKey)
            {
                item.scenePath = FilePath(value);
            }
            else
            {
                if (item.additionalParams.Get() == nullptr)
                {
                    item.additionalParams.ConstructInplace();
                }

                item.additionalParams->SetString(key, value);
            }
        }

        if (item.itemName.IsValid() == false || item.scenePath.IsEmpty() == true)
        {
            errorFound = true;
        }

        duplicatesFound |= (items->insert(item).second == false);
    }
}

void SlotSystem::ItemsCache::XmlConfigParser::OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName)
{
}

void SlotSystem::ItemsCache::XmlConfigParser::OnFoundCharacters(const String& chars)
{
}

bool SlotSystem::ItemsCache::XmlConfigParser::IsErrorFound() const
{
    return errorFound;
}

bool SlotSystem::ItemsCache::XmlConfigParser::IsDublicatesFound() const
{
    return duplicatesFound;
}

void SlotSystem::ItemsCache::LoadConfigFile(const FilePath& configPath)
{
    String extension = configPath.GetExtension();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension == ".yaml")
    {
        LoadYamlConfig(configPath);
    }
    else if (extension == ".xml")
    {
        LoadXmlConfig(configPath);
    }
    else
    {
        Logger::Error("Unknown slot config file extension %s", configPath.GetAbsolutePathname().c_str());
    }
}

void SlotSystem::ItemsCache::LoadYamlConfig(const FilePath& configPath)
{
    ScopedPtr<YamlParser> parser(YamlParser::Create(configPath));
    if (!parser)
    {
        Logger::Error("Couldn't parse yaml file %s", configPath.GetAbsolutePathname().c_str());
        return;
    }

    YamlNode* rootNode = parser->GetRootNode();
    if (rootNode == nullptr)
    {
        Logger::Error("Configuration file %s is empty", configPath.GetAbsolutePathname().c_str());
        return;
    }

    String nameKey("Name");
    String typeKey("Type");
    String pathKey("Path");

    bool incorrectItemsFound = false;
    bool duplicatesFound = false;
    Set<Item, ItemLess>& items = cachedItems[configPath.GetAbsolutePathname()];

    const DAVA::Vector<DAVA::YamlNode*>& yamlNodes = rootNode->AsVector();
    size_t propertiesCount = yamlNodes.size();
    for (YamlNode* currentNode : yamlNodes)
    {
        uint32 fieldsCount = currentNode->GetCount();

        Item newItem;
        for (uint32 fieldIndex = 0; fieldIndex < fieldsCount; ++fieldIndex)
        {
            const YamlNode* fieldNode = currentNode->Get(fieldIndex);
            const String& key = currentNode->GetItemKeyName(fieldIndex);
            if (fieldNode->GetType() == YamlNode::TYPE_STRING)
            {
                if (key == nameKey && fieldNode->GetType() == YamlNode::TYPE_STRING)
                {
                    newItem.itemName = FastName(fieldNode->AsString());
                }
                else if (key == pathKey)
                {
                    String path = fieldNode->AsString();
                    newItem.scenePath = FilePath(path);
                }
                else if (key == typeKey)
                {
                    newItem.type = FastName(fieldNode->AsString());
                }
                else
                {
                    if (nullptr == newItem.additionalParams)
                    {
                        newItem.additionalParams.ConstructInplace();
                    }

                    newItem.additionalParams->SetString(key, fieldNode->AsString());
                }
            }
        }

        bool isItemValid = newItem.itemName.IsValid() && newItem.scenePath.IsEmpty() == false;
        if (isItemValid == false)
        {
            incorrectItemsFound = true;
        }
        else
        {
            duplicatesFound |= (items.insert(newItem).second == false);
        }
    }

    if (incorrectItemsFound == true)
    {
        Logger::Error("Yaml parsing error. Config file %s contains incomplete items", configPath.GetAbsolutePathname().c_str());
    }

    if (duplicatesFound == true)
    {
        Logger::Error("Yaml parsing error. Config file %s contains duplicated items", configPath.GetAbsolutePathname().c_str());
    }
}

void SlotSystem::ItemsCache::LoadXmlConfig(const FilePath& configPath)
{
    Set<Item, ItemLess>& items = cachedItems[configPath.GetAbsolutePathname()];
    ItemsCache::XmlConfigParser parser(&items);
    XMLParser::ParseFile(configPath, &parser);
}

const SlotSystem::ItemsCache::Item* SlotSystem::ItemsCache::LookUpItem(const FilePath& configPath, FastName itemName)
{
    String absolutePath = configPath.GetAbsolutePathname();
    auto configIter = cachedItems.find(absolutePath);
    if (configIter == cachedItems.end())
    {
        LoadConfigFile(configPath);
        configIter = cachedItems.find(absolutePath);
    }

    if (configIter == cachedItems.end())
    {
        return nullptr;
    }

    Item key;
    key.itemName = itemName;

    auto itemIter = configIter->second.find(key);
    if (itemIter == configIter->second.end())
    {
        return nullptr;
    }

    return &(*itemIter);
}

Vector<SlotSystem::ItemsCache::Item> SlotSystem::ItemsCache::GetItems(const FilePath& configPath)
{
    Vector<Item> result;
    if (configPath.IsEmpty())
    {
        return result;
    }

    String absolutePath = configPath.GetAbsolutePathname();
    auto configIter = cachedItems.find(absolutePath);
    if (configIter == cachedItems.end())
    {
        LoadConfigFile(configPath);
        configIter = cachedItems.find(absolutePath);
    }

    if (configIter != cachedItems.end())
    {
        std::copy(configIter->second.begin(), configIter->second.end(), std::back_inserter(result));
    }

    return result;
}

bool SlotSystem::ItemsCache::ItemLess::operator()(const Item& item1, const Item& item2) const
{
    return item1.itemName < item2.itemName;
}

void SlotSystem::ExternalEntityLoader::SetScene(Scene* scene)
{
    if (scene == nullptr)
    {
        Reset();
    }
    this->scene = scene;
}

void SlotSystem::ExternalEntityLoader::AddEntity(Entity* parent, Entity* child)
{
    parent->AddNode(child);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              SlotSystem                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SlotSystem::SlotSystem(Scene* scene)
    : SceneSystem(scene)
    , sharedCache(new ItemsCache())
    , externalEntityLoader(new AsyncSlotExternalLoader())
{
    deletePending.reserve(4);
}

SlotSystem::~SlotSystem()
{
    DVASSERT(externalEntityLoader != nullptr);
    externalEntityLoader->Reset();
}

void SlotSystem::SetSharedCache(std::shared_ptr<ItemsCache> cache)
{
    sharedCache = cache;
}

Vector<SlotSystem::ItemsCache::Item> SlotSystem::GetItems(const FilePath& configPath)
{
    return sharedCache->GetItems(configPath);
}

void SlotSystem::SetExternalEntityLoader(std::shared_ptr<ExternalEntityLoader> externalEntityLoader_)
{
    DVASSERT(externalEntityLoader_ != nullptr);
    DVASSERT(externalEntityLoader != nullptr);
    externalEntityLoader->SetScene(nullptr);
    externalEntityLoader = externalEntityLoader_;
    externalEntityLoader->SetScene(GetScene());
}

void SlotSystem::UnregisterEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(deletePending, entity);

    auto loadedIter = std::find(loadedEntities.begin(), loadedEntities.end(), entity);
    if (loadedIter != loadedEntities.end())
    {
        DVASSERT(components.size() == states.size());
        DVASSERT(components.size() >= loadedEntities.size());

        size_t index = std::distance(loadedEntities.begin(), loadedIter);
        size_t loadedLastIndex = loadedEntities.size() - 1;
        states[index] = eSlotState::NOT_LOADED;
        std::swap(components[index], components[loadedLastIndex]);
        std::swap(loadedEntities[index], loadedEntities[loadedLastIndex]);
        std::swap(states[index], states[loadedLastIndex]);

        loadedEntities.pop_back();
    }
    SceneSystem::UnregisterEntity(entity);
}

void SlotSystem::AddEntity(Entity* entity)
{
    uint32 count = entity->GetComponentCount(Component::SLOT_COMPONENT);
    for (uint32 i = 0; i < count; ++i)
    {
        AddComponent(entity, entity->GetComponent(Component::SLOT_COMPONENT, i));
    }
}

void SlotSystem::RemoveEntity(Entity* entity)
{
    uint32 count = entity->GetComponentCount(Component::SLOT_COMPONENT);
    for (uint32 i = 0; i < count; ++i)
    {
        RemoveComponent(entity, entity->GetComponent(Component::SLOT_COMPONENT, i));
    }
}

void SlotSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::SLOT_COMPONENT);
    components.push_back(static_cast<SlotComponent*>(component));
    states.push_back(eSlotState::NOT_LOADED);
}

void SlotSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType() == Component::SLOT_COMPONENT);
    size_t index = GetComponentIndex(static_cast<SlotComponent*>(component));
    size_t lastIndex = components.size() - 1;
    DVASSERT(components.size() == states.size());

    std::swap(components[index], components[lastIndex]);
    std::swap(states[index], states[lastIndex]);
    components.pop_back();
    states.pop_back();

    if (index < loadedEntities.size())
    {
        deletePending.push_back(loadedEntities[index]);
        std::swap(loadedEntities[index], loadedEntities[loadedEntities.size() - 1]);
        loadedEntities.pop_back();
    }
}

void SlotSystem::Process(float32 timeElapsed)
{
    Vector<Entity*> deletePendingCopy = deletePending;
    deletePending.clear();
    if (deletePendingCopy.empty() == false)
    {
        for (Entity* e : deletePendingCopy)
        {
            Entity* parent = e->GetParent();
            parent->RemoveNode(e);
        }
    }

    for (size_t i = 0; i < loadedEntities.size(); ++i)
    {
        loadedEntities[i]->SetLocalTransform(GetResultTranform(components[i]));
    }

    DVASSERT(externalEntityLoader != nullptr);
    externalEntityLoader->Process(timeElapsed);
}

void SlotSystem::AttachItemToSlot(Entity* rootEntity, FastName slotName, FastName itemName)
{
    uint32 slotsCount = rootEntity->GetComponentCount(Component::SLOT_COMPONENT);
    for (uint32 i = 0; i < slotsCount; ++i)
    {
        SlotComponent* slotComponent = static_cast<SlotComponent*>(rootEntity->GetComponent(Component::SLOT_COMPONENT, i));
        if (slotComponent->GetSlotName() == slotName)
        {
            AttachItemToSlot(slotComponent, itemName);
        }
    }

    uint32 childCount = rootEntity->GetChildrenCount();
    for (uint32 childIndex = 0; childIndex < childCount; ++childIndex)
    {
        AttachItemToSlot(rootEntity->GetChild(childIndex), slotName, itemName);
    }
}

Entity* SlotSystem::AttachItemToSlot(SlotComponent* component, FastName itemName)
{
    UnloadItem(component);

    const FilePath& configPath = component->GetConfigFilePath();
    uint32 filtersCount = component->GetTypeFiltersCount();

    const ItemsCache::Item* item = sharedCache->LookUpItem(configPath, itemName);
#if defined(__DAVAENGINE_DEBUG__)
    uint32 typeFiltersCount = component->GetTypeFiltersCount();
    bool filterFound = (typeFiltersCount == 0);
    for (uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
    {
        if (component->GetTypeFilter(i) == item->type)
        {
            filterFound = true;
            break;
        }
    }
    DVASSERT(filterFound == true);
#endif

    if (item == nullptr)
    {
        Logger::Error("Couldn't find item %s in config file %s for slot %s: ", itemName.c_str(),
                      component->GetConfigFilePath().GetAbsolutePathname().c_str(),
                      component->GetSlotName().c_str());
        size_t index = GetComponentIndex(component);
        states[index] = eSlotState::LOADING_FAILED;
        return nullptr;
    }

    Entity* slotRootEntity = new Entity();
    DVASSERT(externalEntityLoader != nullptr);
    externalEntityLoader->Load(RefPtr<Entity>::ConstructWithRetain(slotRootEntity), item->scenePath, [this, component, itemName](String&& message)
                               {
                                   auto iter = std::find(components.begin(), components.end(), component);
                                   if (iter == components.end())
                                   {
                                       return;
                                   }

                                   size_t index = std::distance(components.begin(), iter);
                                   // if index >= loadedEntities.size(), root for loaded item has been already remove from scene
                                   if (index < loadedEntities.size())
                                   {
                                       if (message.empty() == false)
                                       {
                                           // "Component" was found in "components". This means that component still in system and pointer is valid
                                           Logger::Error("Loading item %s to slot %s failed: %s", itemName.c_str(), component->GetSlotName(), message.c_str());
                                           states[index] = eSlotState::LOADING_FAILED;
                                       }
                                       else
                                       {
                                           states[index] = eSlotState::LOADED;
                                       }
                                   }
                               });
    externalEntityLoader->AddEntity(component->GetEntity(), slotRootEntity);
    AttachEntityToSlotImpl(component, slotRootEntity, item->itemName, eSlotState::LOADING);
    return slotRootEntity;
}

void SlotSystem::AttachEntityToSlot(SlotComponent* component, Entity* entity, FastName itemName)
{
    UnloadItem(component);
    DVASSERT(component->GetEntity() != nullptr);
    DVASSERT(component->GetEntity()->GetScene() == GetScene());

    Entity* parentEntity = component->GetEntity();
    externalEntityLoader->AddEntity(parentEntity, entity);
    AttachEntityToSlotImpl(component, entity, itemName, eSlotState::LOADED);
}

Entity* SlotSystem::LookUpLoadedEntity(SlotComponent* component) const
{
    DVASSERT(components.size() == states.size());
    DVASSERT(components.size() >= loadedEntities.size());

    size_t index = GetComponentIndex(component);
    ;
    if (index < loadedEntities.size())
    {
        return loadedEntities[index];
    }

    return nullptr;
}

SlotComponent* SlotSystem::LookUpSlot(Entity* entity) const
{
    DVASSERT(components.size() == states.size());
    DVASSERT(components.size() >= loadedEntities.size());

    auto iter = std::find(loadedEntities.begin(), loadedEntities.end(), entity);
    if (iter != loadedEntities.end())
    {
        return components[std::distance(loadedEntities.begin(), iter)];
    }

    return nullptr;
}

Matrix4 SlotSystem::GetJointTransform(SlotComponent* component) const
{
    DVASSERT(component->GetEntity()->GetScene() == GetScene());
    Matrix4 jointTransform;
    FastName boneName = component->GetJointName();
    if (boneName.IsValid())
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
        DVASSERT(skeleton != nullptr);
        uint16 jointId = skeleton->GetJointId(boneName);
        DVASSERT(jointId != SkeletonComponent::INVALID_JOINT_INDEX);
        const SkeletonComponent::JointTransform& transform = skeleton->GetObjectSpaceTransform(jointId);
        jointTransform = transform.orientation.GetMatrix();
        jointTransform.SetTranslationVector(transform.position);

        jointTransform *= Matrix4::MakeScale(Vector3(transform.scale, transform.scale, transform.scale));
    }

    return jointTransform;
}

DAVA::Matrix4 SlotSystem::GetResultTranform(SlotComponent* component) const
{
    DVASSERT(component->GetEntity()->GetScene() == GetScene());
    FastName boneName = component->GetJointName();
    if (boneName.IsValid())
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
        DVASSERT(skeleton != nullptr);
        uint16 jointId = skeleton->GetJointId(boneName);
        DVASSERT(jointId != SkeletonComponent::INVALID_JOINT_INDEX);
        const SkeletonComponent::JointTransform& transform = skeleton->GetObjectSpaceTransform(jointId);
        Matrix4 jointTransform = transform.orientation.GetMatrix();
        jointTransform.SetTranslationVector(transform.position);

        jointTransform *= Matrix4::MakeScale(Vector3(transform.scale, transform.scale, transform.scale));

        return jointTransform * component->GetAttachmentTransform();
    }

    return component->GetAttachmentTransform();
}

void SlotSystem::SetScene(Scene* scene)
{
    SceneSystem::SetScene(scene);
    if (nullptr != externalEntityLoader)
    {
        externalEntityLoader->SetScene(scene);
    }
}

void SlotSystem::AttachEntityToSlotImpl(SlotComponent* component, Entity* entity, FastName itemName, SlotSystem::eSlotState state)
{
    component->loadedItemName = itemName;
    entity->SetName(component->GetSlotName());

    size_t index = GetComponentIndex(component);
    DVASSERT(index >= loadedEntities.size());

    size_t targetIndex = loadedEntities.size();
    loadedEntities.push_back(entity);
    states[targetIndex] = state;
    std::swap(components[index], components[targetIndex]);
    std::swap(states[index], states[targetIndex]);
}

void SlotSystem::UnloadItem(SlotComponent* component)
{
    component->loadedItemName = FastName();
    size_t index = GetComponentIndex(component);
    if (index < loadedEntities.size())
    {
        deletePending.push_back(loadedEntities[index]);
        RemoveExchangingWithLast(loadedEntities, index);
        states[index] = eSlotState::NOT_LOADED;

        std::swap(components[index], components[loadedEntities.size()]);
        std::swap(states[index], states[loadedEntities.size()]);
    }
}

SlotSystem::eSlotState SlotSystem::GetSlotState(const SlotComponent* component) const
{
    return states[GetComponentIndex(component)];
}

size_t SlotSystem::GetComponentIndex(const SlotComponent* component) const
{
    auto iter = std::find(components.begin(), components.end(), component);
    DVASSERT(iter != components.end(), "[SlotSystem::GetSlotState] Input component doesn't atteched to current scene");

    return std::distance(components.begin(), iter);
}

} // namespace DAVA
