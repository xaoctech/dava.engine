#pragma once

#include "BotTaskComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
class WaitTaskComponent final : public BotTaskComponent
{
public:
    enum class Type : uint8
    {
        DELAY,
        TIMESTAMP
    };

    WaitTaskComponent()
    {
    }
    WaitTaskComponent(Type type, float time);
    Component* Clone(Entity* toEntity) override;

private:
    float time = 0.f;
    Type type = Type::DELAY;

    friend class BotTaskSystem;

    DAVA_VIRTUAL_REFLECTION(WaitTaskComponent, BotTaskComponent, Component);
};

class CompositeTaskComponent final : public BotTaskComponent
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
}