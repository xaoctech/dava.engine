#pragma once

#include <Base/Set.h>
#include <Scene3D/Components/CameraComponent.h>

#include <NetworkCore/NetworkTypes.h>
#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

#include <Physics/DynamicBodyComponent.h>

namespace DAVA
{
class Entity;
class Scene;
class UDPServer;
class PolygonGroup;
class IServer;
}

class GameModeSystemPhysics final : public DAVA::INetworkInputSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(GameModeSystemPhysics, DAVA::INetworkInputSimulationSystem);

public:
    GameModeSystemPhysics(DAVA::Scene* scene);

    // SceneSystem overrides
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override;

    // BaseSimulationSystem overrides
    void Simulate(DAVA::Entity* entity) override;

    // INetworkInputSimulationSystem overrides
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;

private:
    void CreateFloorAndWalls();
    void CreateEntities();
    void OnEntityReplicated(DAVA::Entity* entity);
    void ScheduleMagneteForces(DAVA::Entity* entity);
    DAVA::PolygonGroup* CreateCubePolygonGroup(DAVA::float32 size) const;
    DAVA::Entity* CreateSmallCube(DAVA::Entity* replicatedGhost) const;
    DAVA::Entity* CreateBigCube(DAVA::Entity* replicatedGhost) const;

    DAVA::Scene* scene;
    DAVA::IServer* server;
    DAVA::NetworkPlayerID playerId;
    DAVA::Camera* camera;
    DAVA::Texture* cubesTexture;

    DAVA::Set<DAVA::Entity*> smallCubes;
    DAVA::Entity* bigCube = nullptr;
    DAVA::ComponentGroup<DAVA::DynamicBodyComponent>* dynamicBodies = nullptr;
};
