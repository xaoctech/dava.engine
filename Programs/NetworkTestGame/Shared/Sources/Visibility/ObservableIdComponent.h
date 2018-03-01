#pragma once

#include <NetworkCore/NetworkTypes.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

using ObservableId = DAVA::uint16;
constexpr ObservableId MAX_OBSERVABLES_COUNT = DAVA::MAX_NETWORK_VISIBLE_ENTITIES_COUNT;

class ObservableIdComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ObservableIdComponent, DAVA::Component);

    ObservableIdComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    ObservableId id;
};
