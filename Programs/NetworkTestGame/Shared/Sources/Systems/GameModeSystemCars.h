#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"
#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class Scene;
class Entity;
class CameraComponent;
}

class GameModeSystemCars final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameModeSystemCars, DAVA::SceneSystem);

    GameModeSystemCars(DAVA::Scene* scene);

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override;
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void OnClientConnected(const DAVA::Responder& responder);

private:
    DAVA::IServer* server;
    DAVA::Vector<const DAVA::Responder*> connectedResponders;

    DAVA::UnorderedSet<DAVA::Entity*> cars;
    DAVA::UnorderedSet<DAVA::Entity*> switches;
    DAVA::float32 countdown = 0.f;
    DAVA::Entity* focusedCar;
    DAVA::CameraComponent* cameraComponent;
};
