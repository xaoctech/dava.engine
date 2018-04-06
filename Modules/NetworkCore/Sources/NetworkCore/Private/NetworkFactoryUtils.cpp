#include "NetworkCore/NetworkFactoryUtils.h"
#include <NetworkCore/Scene3D/Components/NetworkFactoryComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include "Utils/Utils.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Logger/Logger.h"

#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
const UnorderedMap<FastName, EntityCfg::Domain> EntityConfigManager::DomainFromName =
{
  { FastName("Server"), EntityCfg::Domain::Server },
  { FastName("Client"), EntityCfg::Domain::Client },
  { FastName("ClientOwner"), EntityCfg::Domain::ClientOwner },
  { FastName("ClientNotOwner"), EntityCfg::Domain::ClientNotOwner }
};

const UnorderedMap<FastName, M::Privacy> EntityConfigManager::PrivacyFromName =
{
  { FastName("ClientOwner"), M::Privacy::PRIVATE },
  { FastName("ClientNotOwner"), M::Privacy::NOT_OWNER },
  { FastName("Client"), M::Privacy::PUBLIC },
};

EntityConfigManager::EntityConfigManager()
{
    for (const FilePath& path : FileSystem::Instance()->EnumerateFilesInDirectory(FilePath("~res:/cfg/")))
    {
        if (".yaml" == path.GetExtension())
        {
            ReadYaml(path);
        }
    }
}

const EntityCfg* EntityConfigManager::GetEntityCfg(const String& name) const
{
    const auto& findIt = entityCfgCache.find(FastName(name));
    if (findIt != entityCfgCache.end())
    {
        return &findIt->second;
    }

    return nullptr;
}

void EntityConfigManager::ReadYaml(const FilePath& filePath)
{
    RefPtr<YamlParser> parser(YamlParser::Create(filePath));
    DVASSERT(parser && parser.Valid());

    YamlNode* rootNode = parser->GetRootNode();
    DVASSERT(rootNode);

    const uint32 numberOfEntities = rootNode->GetCount();
    DVASSERT(numberOfEntities);

    const String fileName = filePath.GetBasename();
    for (uint32 entityIdx = 0; entityIdx < numberOfEntities; ++entityIdx)
    {
        const YamlNode* entityNode = rootNode->Get(entityIdx);
        ReadEntity(fileName, entityNode);
    }
}

void EntityConfigManager::ReadEntity(const String& fileName, const YamlNode* entityNode)
{
    using D = EntityCfg::Domain;

    const YamlNode* prototypeNode = entityNode->Get("Prototype");
    EntityCfg entityCfg;
    if (prototypeNode)
    {
        String prototypeName = prototypeNode->AsString();
        if (prototypeName.find("/") == String::npos)
        {
            prototypeName = fileName + "/" + prototypeName;
        }
        const EntityCfg* protoCfg = GetEntityCfg(prototypeName);
        DVASSERT(protoCfg);
        entityCfg = *protoCfg;
    }

    entityCfg.domainMask = ReadDomainMask(entityNode, "Domains", entityCfg.domainMask);
    const YamlNode* componentsNode = entityNode->Get("Components");
    if (componentsNode)
    {
        const size_t numberOfComponents = componentsNode->GetCount();
        for (size_t compIdx = 0; compIdx < numberOfComponents; ++compIdx)
        {
            const YamlNode* compNode = componentsNode->Get(compIdx);
            const String& compName = componentsNode->GetItemKeyName(compIdx);
            const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(compName);
            DVASSERT(reflectedType);
            if (reflectedType->GetType()->Is<NetworkPredictComponent>())
            {
                /* Should be created auto */
                continue;
            }

            EntityCfg::ComponentCfg& componentCfg = entityCfg.components[reflectedType];
            componentCfg.domainMask = ReadDomainMask(compNode, "Domains", entityCfg.domainMask);
            DVASSERT((entityCfg.domainMask & componentCfg.domainMask) == componentCfg.domainMask, "Component has invalid domain");
            componentCfg.predictDomainMask = ReadDomainMask(compNode, "Predict", 0);
            DVASSERT((componentCfg.domainMask & componentCfg.predictDomainMask) == componentCfg.predictDomainMask, "Component has invalid prediction domain");
            componentCfg.replicationPrivacy = ReadReplicationPrivacy(compNode, "Replicate", M::Privacy::SERVER_ONLY);
            DVASSERT(componentCfg.replicationPrivacy != M::Privacy::NOT_OWNER, "Component has invalid replication tag");
            const uint8 replicateMask = ReadDomainMask(compNode, "Replicate", 0);
            DVASSERT(!((componentCfg.domainMask & ~componentCfg.predictDomainMask) & replicateMask), "Component has replicate and domain tags both");

            const YamlNode* fieldsNode = compNode->Get("Fields");
            if (fieldsNode)
            {
                size_t numberOfFields = fieldsNode->GetCount();
                componentCfg.fields.reserve(numberOfFields);
                for (size_t fieldIdx = 0; fieldIdx < numberOfFields; ++fieldIdx)
                {
                    FastName fieldName(fieldsNode->GetItemKeyName(fieldIdx));
                    const YamlNode* fieldNode = fieldsNode->Get(fieldIdx);

                    const ReflectedStructure::Field* field = GetField(reflectedType, fieldName);
                    DVASSERT(field);
                    const String& strValue = fieldNode->AsString();
                    //                    TODO: Use this method after completed reflection improvement.
                    //                    Any value = Any(strValue).Cast(field->valueWrapper->GetValueType());
                    Any value = fieldNode->AsAny(field);
                    componentCfg.fields.push_back({ fieldName, value });
                }
            }
        }
    }

    const YamlNode* modelsNode = entityNode->Get("Models");
    if (modelsNode)
    {
        const uint32 numberOfModels = modelsNode->GetCount();
        entityCfg.models.reserve(numberOfModels);
        uint8 mask = 0;
        for (uint32 modelIdx = 0; modelIdx < numberOfModels; ++modelIdx)
        {
            const YamlNode* modelNode = modelsNode->Get(modelIdx);
            const uint8 modelDomain = ReadDomainMask(modelNode, "Domains", entityCfg.domainMask);
            DVASSERT((mask & modelDomain) == 0, "Intersect model masks");
            const YamlNode* resourceNode = modelNode->Get("Resource");
            DVASSERT(resourceNode);
            FilePath modelFileName("~res:/" + resourceNode->AsString());
            bool isExists = GetEngineContext()->fileSystem->Exists(modelFileName);
            DVASSERT(isExists);
            ScopedPtr<Scene> scene(new Scene(0));
            SceneFileV2::eError err = scene->LoadScene(modelFileName);
            DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
            DVASSERT(scene->GetChildrenCount() == 1);
            Entity* model = scene->GetChild(0)->Clone();
            model->SetName("Model");
            entityCfg.models.push_back({ model, modelDomain });
            mask |= modelDomain;
        }
    }

    const YamlNode* nameNode = entityNode->Get("Name");
    DVASSERT(nameNode);
    const String& entityName = nameNode->AsString();
    entityCfgCache.emplace(FastName(fileName + "/" + entityName), std::move(entityCfg));
}

EntityCfg::Domain EntityConfigManager::GetDomainByName(FastName name)
{
    auto findIt = DomainFromName.find(name);
    if (findIt == DomainFromName.end())
    {
        return EntityCfg::Domain::None;
    }
    return findIt->second;
}

M::Privacy EntityConfigManager::GetPrivacyByName(FastName name)
{
    auto findIt = PrivacyFromName.find(name);
    if (findIt == PrivacyFromName.end())
    {
        return M::Privacy::SERVER_ONLY;
    }
    return findIt->second;
}

M::Privacy EntityConfigManager::GetPrivacyByDomain(uint8 domainMask)
{
    if (domainMask & EntityCfg::Domain::ClientNotOwner)
        return M::Privacy::PUBLIC;

    if (domainMask & EntityCfg::Domain::ClientOwner)
        return M::Privacy::PRIVATE;

    return M::Privacy::SERVER_ONLY;
}

const ReflectedStructure::Field* EntityConfigManager::GetField(const ReflectedType* currRefType, FastName name)
{
    auto key = std::make_pair(currRefType, name);
    auto findIt = fieldCache.find(key);
    if (findIt != fieldCache.end())
    {
        return findIt->second;
    }

    const ReflectedStructure* structure = currRefType->GetStructure();
    if (structure)
    {
        for (auto& f : structure->fields)
        {
            if (f->name == name)
            {
                fieldCache.emplace(key, f.get());
                return f.get();
            }
        }
    }

    const TypeInheritance* inheritance = currRefType->GetType()->GetInheritance();
    if (inheritance)
    {
        const Vector<TypeInheritance::Info>& baseTypesInfo = inheritance->GetBaseTypes();
        for (auto& baseInfo : baseTypesInfo)
        {
            const ReflectedType* baseRefType = ReflectedTypeDB::GetByType(baseInfo.type);
            const ReflectedStructure::Field* field = GetField(baseRefType, name);
            if (field)
            {
                return field;
            }
        }
    }

    fieldCache.emplace(key, nullptr);
    return nullptr;
};

uint8 EntityConfigManager::ReadDomainMask(const YamlNode* node, const String& sectionName, uint8 defaultValue)
{
    const YamlNode* domainsNode = node->Get(sectionName);
    if (!domainsNode)
        return defaultValue;

    uint8 result = 0;
    const size_t numberOfDomains = domainsNode->GetCount();
    for (size_t idx = 0; idx < numberOfDomains; ++idx)
    {
        FastName domainName = domainsNode->Get(idx)->AsFastName();
        EntityCfg::Domain domain = GetDomainByName(domainName);
        DVASSERT(domain != EntityCfg::Domain::None);
        result |= domain;
    }
    return result;
}

M::Privacy EntityConfigManager::ReadReplicationPrivacy(const YamlNode* node, const String& sectionName, M::Privacy defaultValue)
{
    const YamlNode* privacyNode = node->Get(sectionName);
    if (!privacyNode)
        return defaultValue;

    uint8 result = 0;
    const size_t numberOfPrivacy = privacyNode->GetCount();
    for (size_t idx = 0; idx < numberOfPrivacy; ++idx)
    {
        FastName domainName = privacyNode->Get(idx)->AsFastName();
        M::Privacy privacy = GetPrivacyByName(domainName);
        DVASSERT(privacy != M::Privacy::SERVER_ONLY);
        result |= privacy;
    }

    return static_cast<M::Privacy>(result);
}

NetworkFactoryComponent* CreateEntityWithFactoryComponent(const String& name, NetworkID replicationId)
{
    Entity* entity = new Entity();
    NetworkFactoryComponent* factoryComponent = new NetworkFactoryComponent();
    factoryComponent->name = name;
    factoryComponent->replicationId = replicationId;

    entity->AddComponent(factoryComponent);
    return factoryComponent;
}
} //namespace DAVA
