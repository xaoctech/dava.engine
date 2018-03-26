#pragma once

#include <Base/BaseTypes.h>
#include <Math/Quaternion.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NetworkMovementSystem;
class NetworkMovementComponent : public Component
{
    friend class NetworkMovementSystem;

    DAVA_VIRTUAL_REFLECTION(NetworkMovementComponent, Component);

public:
    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    struct MoveState
    {
        uint32 frameId = 0;
        Vector3 translation;
        Quaternion rotation;
    };

    size_t HistoryGetSize();
    MoveState& HistoryAt(size_t index);
    MoveState& HistoryBack();
    void HistoryResize(size_t size);
    void HistoryPushBack(MoveState&&);

    size_t interpolationHistoryPushBackPos = 0;
    Vector<MoveState> interpolationHistory;

    MoveState smoothCorrection;
    float32 smoothCorrectionTimeLeft = 0.0f;
};
}
