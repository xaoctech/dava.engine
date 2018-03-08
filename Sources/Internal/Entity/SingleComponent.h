#pragma once

#include "Component.h"
#include "Scene3D/Entity.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class SingleComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override final;
    virtual ~SingleComponent(){};

protected:
    DAVA_VIRTUAL_REFLECTION(SingleComponent, Component);
};

/**
    Single component which contains data about events happened during a frame.
    It's 'clearable' since every frame this data should be cleared and filled with new data from a new frame.

    `Scene` is responsible for clearing all such single components.
*/
class ClearableSingleComponent : public SingleComponent
{
    // For clearing
    friend class Scene;

public:
    /**
        Describes how data, this single component contains, is used.
        It defines at which moment this component will be cleared.
    */
    enum class Usage
    {
        /**
            Data is used by fixed and non-fixed processes.
            Cleared at the end of a frame, after all fixed and non-fixed processes are called.
        */
        AllProcesses,

        /**
            Data is used only by systems with fixed process.
            Cleared after all fixed processes and before first non-fixed process is called.
        */
        FixedProcesses
    };

public:
    explicit ClearableSingleComponent(Usage usage);
    ClearableSingleComponent(const ClearableSingleComponent&) = delete;
    ClearableSingleComponent& operator=(const ClearableSingleComponent&) = delete;

protected:
    DAVA_VIRTUAL_REFLECTION(ClearableSingleComponent, SingleComponent);

private:
    /** Clear this component's per-frame data. */
    virtual void Clear() = 0;

    Usage GetUsage() const
    {
        return usage;
    }

private:
    const Usage usage;
};
}
