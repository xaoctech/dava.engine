#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SingleComponent.h>
#include <Reflection/Reflection.h>
#include <NetworkCore/Private/NetworkSerialization.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class NetworkEntitiesSingleComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(NetworkEntitiesSingleComponent, SingleComponent);

public:
    NetworkEntitiesSingleComponent();
    virtual ~NetworkEntitiesSingleComponent(){};

    void RegisterEntity(NetworkID networkID, Entity*);
    void UnregisterEntity(NetworkID networkID);

    Entity* FindByID(NetworkID networkID) const;

private:
    UnorderedMap<NetworkID, Entity*> networkEntities;
    NetworkID nextUniqueEntityID;
};
}
