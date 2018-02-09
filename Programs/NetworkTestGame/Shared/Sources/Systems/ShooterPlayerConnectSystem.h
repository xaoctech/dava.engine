#pragma once

#include <Base/Vector.h>
#include <Entity/SceneSystem.h>

#include <NetworkCore/UDPTransport/UDPServer.h>

namespace DAVA
{
class Scene;
}

// Responsible for listening to new connections and creating entities with player role,
// which are to be filled by ShooterEntityFillSystem later
class ShooterPlayerConnectSystem final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterPlayerConnectSystem, DAVA::SceneSystem);

    ShooterPlayerConnectSystem(DAVA::Scene* scene);
    void OnPlayerConnected(const DAVA::Responder& responder);
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;

private:
    void AddPlayerToScene(const DAVA::Responder& responder);

private:
    DAVA::Vector<const DAVA::Responder*> newPlayers; // List of players connected since last Process call
};