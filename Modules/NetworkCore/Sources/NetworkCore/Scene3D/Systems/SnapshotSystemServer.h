#pragma once

#include "NetworkCore/Scene3D/Systems/SnapshotSystemBase.h"

namespace DAVA
{
class SnapshotSystemServer final : public SnapshotSystemBase
{
public:
    DAVA_VIRTUAL_REFLECTION(SnapshotSystemServer, SnapshotSystemBase);

    using SnapshotSystemBase::SnapshotSystemBase;

private:
    void ProcessFixed(float32 timeElapsed) override;
};
} // namespace DAVA
