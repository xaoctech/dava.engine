#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "Base/FastName.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class NetworkFactoryComponent;
class Scene;
class ReflectedType;

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

    static Domain GetDomainByName(FastName name);
    static M::Privacy GetPrivacyByName(FastName name);
    static M::Privacy GetPrivacyByDomain(uint8 domainMask);

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
    Entity* model = nullptr;

    static const EntityCfg& LoadFromYaml(const String& name);
};

NetworkFactoryComponent* CreateFactoryEntity(const String& name, NetworkID replicationId);

} //namespace DAVA
