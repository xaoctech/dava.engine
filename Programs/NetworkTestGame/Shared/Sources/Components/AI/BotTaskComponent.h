#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

#include <functional>

using namespace DAVA;

enum class BotTaskStatus : uint8
{
    NOT_STARTED,
    IN_PROGRESS,
    SUCCESS,
    FAILURE
};

class BotTaskComponent : public Component
{
public:
    typedef std::function<void(BotTaskComponent*)> TraversalCallback;

    BotTaskComponent()
    {
    }
    virtual void TraverseTaskTree(const TraversalCallback& callback);

    BotTaskStatus GetStatus() const;

protected:
    BotTaskStatus status = BotTaskStatus::NOT_STARTED;

    friend class BotTaskSystem;
};

inline void BotTaskComponent::TraverseTaskTree(const TraversalCallback& callback)
{
    callback(this);
}

inline BotTaskStatus BotTaskComponent::GetStatus() const
{
    return status;
}
