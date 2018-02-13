#pragma once

#include <Entity/SingletonComponent.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class NetworkDeltaSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDeltaSingleComponent, SingletonComponent);

    struct Delta
    {
        uint32 sequenceId;

        NetworkID netEntityId;
        uint32 baseFrameId;
        uint32 frameId;

        const uint8* srcBuff;
        size_t srcSize;

        enum class Status
        {
            PENDING,
            APPLIED,
            SKIPPED
        };

        Status status = Status::PENDING;
    };

    using Deltas = Vector<Delta>;
    Deltas deltas;

    void Clear() override;
};
}
