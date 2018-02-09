#pragma once

#include "BotTaskComponent.h"
#include <Entity/Component.h>
#include <Scene3D/Entity.h>

using namespace DAVA;

class CompositeTaskComponent : public BotTaskComponent
{
public:
    enum class Type : uint8
    {
        AND,
        OR
    };

    CompositeTaskComponent()
    {
    }
    CompositeTaskComponent(Type type, BotTaskComponent* first, BotTaskComponent* second);
    Component* Clone(Entity* toEntity) override;
    void TraverseTaskTree(const TraversalCallback& callback) override;

private:
    BotTaskComponent* first;
    BotTaskComponent* second;
    Type type;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(CompositeTaskComponent, BotTaskComponent, Component);
};
