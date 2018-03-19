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

    void MoveSmothly(const Vector3& positionDelta, const Quaternion& rotationDelta);

    struct MoveState
    {
        uint32 frameId;
        Vector3 translation;
        Quaternion rotation;
    };

    size_t HistoryGetSize();
    MoveState& HistoryAt(size_t index);
    MoveState& HistoryBack();
    void HistoryResize(size_t size);
    void HistoryPushBack(MoveState&&);
    void CorrectionApply(MoveState&&);

    uint32 correctionRule = 0;
    float32 correctionTimeoutSec = 1.0f;

private:
    float32 correctionTimeLeft = 0.0f;
    MoveState correction;

    size_t pushBackPos = 0;
    Vector<MoveState> history;
};
}
