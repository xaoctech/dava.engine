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
struct PairHash
{
    template <typename L, typename R>
    std::size_t operator()(const std::pair<L, R>& x) const
    {
        return std::hash<L>()(x.first) ^ std::hash<R>()(x.second);
    }
};

static const UnorderedMap<FastName, EntityCfg::Domain> gDomainFromName =
{
  { FastName("Server"), EntityCfg::Domain::Server },
  { FastName("Client"), EntityCfg::Domain::Client },
  { FastName("ClientOwner"), EntityCfg::Domain::ClientOwner },
  { FastName("ClientNotOwner"), EntityCfg::Domain::ClientNotOwner }
};

EntityCfg::Domain EntityCfg::GetDomainByName(FastName name)
{
    auto findIt = gDomainFromName.find(name);
    if (findIt == gDomainFromName.end())
    {
        return Domain::None;
    }
    return findIt->second;
}

static const UnorderedMap<FastName, M::Privacy> gPrivacyFromName =
{
  { FastName("ClientOwner"), M::Privacy::OWNER },
  { FastName("ClientNotOwner"), M::Privacy::NOT_OWNER },
  { FastName("Client"), M::Privacy::PUBLIC },
};

M::Privacy EntityCfg::GetPrivacyByName(FastName name)
{
    auto findIt = gPrivacyFromName.find(name);
    if (findIt == gPrivacyFromName.end())
    {
        return M::Privacy::SERVER_ONLY;
    }
    return findIt->second;
}

M::Privacy EntityCfg::GetPrivacyByDomain(uint8 domainMask)
{
    if (domainMask & Domain::ClientNotOwner)
        return M::Privacy::PUBLIC;

    if (domainMask & Domain::ClientOwner)
        return M::Privacy::PRIVATE;

    return M::Privacy::SERVER_ONLY;
}

const ReflectedStructure::Field* GetField(const ReflectedType* currRefType, FastName name)
{
    static UnorderedMap<std::pair<const ReflectedType*, FastName>, const ReflectedStructure::Field*, PairHash> gFieldCache;
    auto key = std::make_pair(currRefType, name);
    auto findIt = gFieldCache.find(key);
    if (findIt != gFieldCache.end())
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
                gFieldCache.emplace(key, f.get());
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

    gFieldCache.emplace(key, nullptr);
    return nullptr;
};

uint8 ReadDomainMask(const YamlNode* node, const String& sectionName, uint8 defaultValue)
{
    const YamlNode* domainsNode = node->Get(sectionName);
    if (!domainsNode)
        return defaultValue;

    uint8 result = 0;
    const size_t numberOfDomains = domainsNode->GetCount();
    for (size_t idx = 0; idx < numberOfDomains; ++idx)
    {
        FastName domainName = domainsNode->Get(idx)->AsFastName();
        EntityCfg::Domain domain = EntityCfg::GetDomainByName(domainName);
        DVASSERT(domain != EntityCfg::Domain::None);
        result |= domain;
    }
    return result;
}

M::Privacy ReadReplicationPrivacy(const YamlNode* node, const String& sectionName, M::Privacy defaultValue)
{
    const YamlNode* privacyNode = node->Get(sectionName);
    if (!privacyNode)
        return defaultValue;

    uint8 result = 0;
    const size_t numberOfPrivacy = privacyNode->GetCount();
    for (size_t idx = 0; idx < numberOfPrivacy; ++idx)
    {
        FastName domainName = privacyNode->Get(idx)->AsFastName();
        M::Privacy privacy = EntityCfg::GetPrivacyByName(domainName);
        DVASSERT(privacy != M::Privacy::SERVER_ONLY);
        result |= privacy;
    }

    return static_cast<M::Privacy>(result);
}

const EntityCfg& EntityCfg::LoadFromYaml(const String& name)
{
    using D = EntityCfg::Domain;
    static UnorderedMap<String, EntityCfg> gEntityCfgStorage;
    EntityCfg& entityCfg = gEntityCfgStorage[name];
    if (entityCfg.domainMask > 0)
    {
        return entityCfg;
    }

    FilePath filename("~res:/" + name + ".yaml");
    bool isExist = GetEngineContext()->fileSystem->Exists(filename);
    DVASSERT(isExist);
    if (!isExist)
    {
        Logger::Error("GetEntityConfig Does not exists (%s)", filename.GetStringValue().c_str());
        return entityCfg;
    }

    RefPtr<YamlParser> parser(YamlParser::Create(filename));
    DVASSERT(parser && parser.Valid());
    if (!parser || !parser.Valid())
    {
        Logger::Error("GetEntityConfig failed (%s)", filename.GetStringValue().c_str());
        return entityCfg;
    }

    YamlNode* rootNode = parser->GetRootNode();
    DVASSERT(rootNode);
    if (!rootNode)
    {
        Logger::Error("GetEntityConfig empty (%s)", filename.GetStringValue().c_str());
        return entityCfg;
    }

    entityCfg.domainMask = ReadDomainMask(rootNode, "Domains", D::Server | D::ClientOwner);
    const YamlNode* componentsNode = rootNode->Get("Components");
    DVASSERT(componentsNode);
    if (!componentsNode)
    {
        Logger::Error("GetEntityConfig no any entities (%s)", filename.GetStringValue().c_str());
        return entityCfg;
    }

    const size_t numberOfComponents = componentsNode->GetCount();
    for (size_t compIdx = 0; compIdx < numberOfComponents; ++compIdx)
    {
        const YamlNode* compNode = componentsNode->Get(compIdx);
        const String& compName = componentsNode->GetItemKeyName(compIdx);
        const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(compName);
        if (!reflectedType)
        {
            Logger::Error("Component:%s not found", compName.c_str());
            DVASSERT(0);
            continue;
        }

        if (reflectedType->GetType()->Is<NetworkPredictComponent>())
        {
            /* Should be created auto */
            continue;
        }

        ComponentCfg& componentCfg = entityCfg.components[reflectedType];
        componentCfg.domainMask = ReadDomainMask(compNode, "Domains", entityCfg.domainMask);
        if ((entityCfg.domainMask & componentCfg.domainMask) != componentCfg.domainMask)
        {
            Logger::Error("Component:%s has invalid domain", compName.c_str());
            DVASSERT(0);
        }

        componentCfg.predictDomainMask = ReadDomainMask(compNode, "Predict", 0);
        if ((componentCfg.domainMask & componentCfg.predictDomainMask) != componentCfg.predictDomainMask)
        {
            Logger::Error("Component:%s has invalid prediction domain", compName.c_str());
            DVASSERT(0);
        }

        componentCfg.replicationPrivacy = ReadReplicationPrivacy(compNode, "Replicate", M::Privacy::SERVER_ONLY);

        const YamlNode* fieldsNode = compNode->Get("Fields");
        if (fieldsNode)
        {
            size_t numberOfFields = fieldsNode->GetCount();
            for (size_t fieldIdx = 0; fieldIdx < numberOfFields; ++fieldIdx)
            {
                FastName fieldName(fieldsNode->GetItemKeyName(fieldIdx));
                const YamlNode* fieldNode = fieldsNode->Get(fieldIdx);

                const ReflectedStructure::Field* field = GetField(reflectedType, fieldName);
                if (field)
                {
                    Any value = fieldNode->AsAny(field);
                    componentCfg.fields.push_back({ fieldName, value });
                }
                else
                {
                    Logger::Error("Component: %s has not field: %s", compName.c_str(), fieldName.c_str());
                    DVASSERT(0);
                }
            }
        }
    }

    const YamlNode* modelNode = rootNode->Get("Model");
    if (modelNode)
    {
        FilePath modelFileName("~res:/" + modelNode->AsString() + ".sc2");
        bool isExists = GetEngineContext()->fileSystem->Exists(modelFileName);
        DVASSERT(isExists);
        if (!isExists)
        {
            Logger::Error("Model does not exists (%s)", modelFileName.GetStringValue().c_str());
            return entityCfg;
        }

        ScopedPtr<Scene> model(new Scene(0));
        SceneFileV2::eError err = model->LoadScene(modelFileName);
        DVASSERT(SceneFileV2::ERROR_NO_ERROR == err);
        entityCfg.model = model->GetEntityByID(1)->Clone();
        entityCfg.model->SetName("Model");
    }

    return entityCfg;
}

NetworkFactoryComponent* CreateFactoryEntity(const String& name, NetworkID replicationId)
{
    Entity* entity = new Entity();
    NetworkFactoryComponent* factoryComponent = new NetworkFactoryComponent();
    factoryComponent->name = name;
    factoryComponent->playerId = replicationId.GetPlayerId();

    const EntityCfg& entityCfg = EntityCfg::LoadFromYaml(name);
    const M::Privacy privacy = EntityCfg::GetPrivacyByDomain(entityCfg.domainMask);
    if (privacy != M::Privacy::SERVER_ONLY)
    {
        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(replicationId);
        replicationComponent->SetForReplication<NetworkFactoryComponent>(privacy);

        for (const auto& componentIt : entityCfg.components)
        {
            const ReflectedType* reflectedType = componentIt.first;
            const EntityCfg::ComponentCfg& componentCfg = componentIt.second;
            if (componentCfg.replicationPrivacy != M::Privacy::SERVER_ONLY)
            {
                replicationComponent->SetForReplication(reflectedType->GetType(), componentCfg.replicationPrivacy);
            }
        }
        entity->AddComponent(replicationComponent);
    }

    entity->AddComponent(factoryComponent);
    entity->SetName(name.c_str());

    return factoryComponent;
}
} //namespace DAVA
