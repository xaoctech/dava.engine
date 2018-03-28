#pragma once

#include <Base/Vector.h>
#include <Entity/SceneSystem.h>

#include <NetworkCore/UDPTransport/UDPServer.h>

namespace DAVA
{
class NetworkServerConnectionsSingleComponent;
} // namespace DAVA

/** Responsible for listening to new connections and creating player's (big cube) entities. */
class CubesPlayerConnectSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(CubesPlayerConnectSystem, DAVA::SceneSystem);

    CubesPlayerConnectSystem(DAVA::Scene* scene);
    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void CreatePlayer(const DAVA::FastName& token) const;

private:
    DAVA::NetworkServerConnectionsSingleComponent* networkServerConnectionsSingleComponent = nullptr;
};