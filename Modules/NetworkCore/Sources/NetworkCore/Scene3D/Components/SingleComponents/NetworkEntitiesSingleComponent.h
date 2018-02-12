#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingletonComponent.h>
#include <Reflection/Reflection.h>
#include <NetworkCore/Private/NetworkSerialization.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class NetworkEntitiesSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkEntitiesSingleComponent, SingletonComponent);

public:
    NetworkEntitiesSingleComponent();
    virtual ~NetworkEntitiesSingleComponent(){};

    NetworkID GetNextUniqueEntityID();
    void RegisterEntity(NetworkID networkID, Entity*);
    void UnregisterEntity(NetworkID networkID);

    Entity* FindByID(NetworkID networkID) const;

private:
    UnorderedMap<NetworkID, Entity*> networkEntities;
    NetworkID nextUniqueEntityID;
};
}
