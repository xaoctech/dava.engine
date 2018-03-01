#pragma once

#include <NetworkCore/NetworkTypes.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

class ObservableComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ObservableComponent, DAVA::Component);

    ObservableComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
};
