#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Private/NetworkBucket.h"

#include <Base/BaseTypes.h>
#include <Base/Introspection.h>
#include <Debug/DVAssert.h>
#include <Entity/Component.h>
#include <Math/Matrix4.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Scene3D/SceneFile/SerializationContext.h>

namespace DAVA
{
class NetworkInputComponent : public Component
{
public:
#pragma pack(push, 1)
    struct Data
    {
        Data() = default;
        uint64 actions = 0;
        uint64 analogStates = 0;
        NetworkPackedQuaternion cameraDelta = { 0 };
        bool operator==(const Data& rhs) const;
        bool operator!=(const Data& rhs) const;
    };
#pragma pack(pop)
    using History = NetworkBucket<Data>;

    NetworkInputComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SaveHistory(uint32 frameId, const NetworkInputComponent::Data& data);

    const History& GetHistory() const;
    History& ModifyHistory();

    bool HasLastCameraOrientation() const;
    void SetLastCameraOrientation(const Quaternion& camOrient);
    const Quaternion& GetLastCameraOrientation() const;

public:
    DAVA_VIRTUAL_REFLECTION(NetworkInputComponent, Component);

private:
    Quaternion lastCameraOrientation;
    bool hasLastCameraOrientation = false;

    History history;

    Quaternion lastCamOrient;

    static const uint8 MAX_HISTORY_SIZE = 64;
};
}
