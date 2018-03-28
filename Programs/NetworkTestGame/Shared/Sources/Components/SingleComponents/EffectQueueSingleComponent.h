#pragma once

#include <NetworkCore/NetworkTypes.h>

#include <Base/BaseTypes.h>
#include <Entity/SingleComponent.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>

struct EffectDescriptor
{
    int resourceId = -1;
    DAVA::float32 duration = 0.f;
    DAVA::Vector3 position;
    DAVA::Quaternion rotation;
    DAVA::NetworkID networkId;
    DAVA::NetworkID parentId;

    EffectDescriptor(int resourceId);

    EffectDescriptor& SetDuration(DAVA::float32 newDuration);
    EffectDescriptor& SetPosition(const DAVA::Vector3& newPosition);
    EffectDescriptor& SetRotation(const DAVA::Quaternion& newRotation);
    EffectDescriptor& SetNetworkId(DAVA::NetworkID newNetworkId);
    EffectDescriptor& SetParentId(DAVA::NetworkID newParentId);
};

class EffectQueueSingleComponent : public DAVA::ClearableSingleComponent
{
    DAVA_VIRTUAL_REFLECTION(EffectQueueSingleComponent, DAVA::ClearableSingleComponent);

public:
    EffectQueueSingleComponent();

    void Clear() override;

    EffectDescriptor& CreateEffect(int resourceId);

    const DAVA::Vector<EffectDescriptor>& GetQueuedEffects() const;

private:
    DAVA::Vector<EffectDescriptor> effects;
};

inline const DAVA::Vector<EffectDescriptor>& EffectQueueSingleComponent::GetQueuedEffects() const
{
    return effects;
}

inline EffectDescriptor::EffectDescriptor(int resourceId)
    : resourceId(resourceId)
{
}

inline EffectDescriptor& EffectDescriptor::SetDuration(DAVA::float32 newDuration)
{
    duration = newDuration;
    return *this;
}

inline EffectDescriptor& EffectDescriptor::SetPosition(const DAVA::Vector3& newPosition)
{
    position = newPosition;
    return *this;
}

inline EffectDescriptor& EffectDescriptor::SetRotation(const DAVA::Quaternion& newRotation)
{
    rotation = newRotation;
    return *this;
}

inline EffectDescriptor& EffectDescriptor::SetNetworkId(DAVA::NetworkID newNetworkId)
{
    networkId = newNetworkId;
    return *this;
}

inline EffectDescriptor& EffectDescriptor::SetParentId(DAVA::NetworkID newParentId)
{
    parentId = newParentId;
    return *this;
}
