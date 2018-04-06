#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "Base/FastName.h"
#include "Reflection/ReflectedMeta.h"
#include "Reflection/ReflectedStructure.h"

namespace DAVA
{
class NetworkFactoryComponent;
class Scene;
class ReflectedType;
class YamlNode;

struct EntityCfg
{
    enum Domain : uint8
    {
        None = 0,
        Server = 1 << 0,
        ClientOwner = 1 << 2,
        ClientNotOwner = 1 << 3,
        Client = ClientOwner | ClientNotOwner,
    };

    struct ComponentCfg
    {
        uint8 domainMask = 0;
        uint8 predictDomainMask = 0;
        M::Privacy replicationPrivacy = M::Privacy::SERVER_ONLY;

        struct FieldValue
        {
            FastName name;
            Any value;
        };

        Vector<FieldValue> fields = {};
    };

    uint8 domainMask = 0;
    UnorderedMap<const ReflectedType*, ComponentCfg> components = {};

    struct ModelDomain
    {
        Entity* model;
        uint8 domainMask;
    };
    Vector<ModelDomain> models;
};

class EntityConfigManager
{
    static const UnorderedMap<FastName, EntityCfg::Domain> DomainFromName;
    static const UnorderedMap<FastName, M::Privacy> PrivacyFromName;
    struct PairHash
    {
        template <typename L, typename R>
        std::size_t operator()(const std::pair<L, R>& x) const
        {
            return std::hash<L>()(x.first) ^ std::hash<R>()(x.second);
        }
    };

public:
    EntityConfigManager();

    const EntityCfg* GetEntityCfg(const String& name) const;
    static EntityCfg::Domain GetDomainByName(FastName name);
    static M::Privacy GetPrivacyByName(FastName name);
    static M::Privacy GetPrivacyByDomain(uint8 domainMask);

private:
    void ReadYaml(const FilePath& filePath);
    void ReadEntity(const String& fileName, const YamlNode* entityNode);
    M::Privacy ReadReplicationPrivacy(const YamlNode* node, const String& sectionName, M::Privacy defaultValue);
    uint8 ReadDomainMask(const YamlNode* node, const String& sectionName, uint8 defaultValue);
    const ReflectedStructure::Field* GetField(const ReflectedType* currRefType, FastName name);

    UnorderedMap<FastName, EntityCfg> entityCfgCache;
    UnorderedMap<std::pair<const ReflectedType*, FastName>, const ReflectedStructure::Field*, EntityConfigManager::PairHash> fieldCache;
};

NetworkFactoryComponent* CreateEntityWithFactoryComponent(const String& name, NetworkID replicationId);

} //namespace DAVA
