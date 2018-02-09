#pragma once

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"
#include "Base/Vector.h"
#include "Scene3D/Components/CameraComponent.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class Scene;
class Entity;
}

class GameModeSystemCharacters final : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(GameModeSystemCharacters, DAVA::SceneSystem);

    GameModeSystemCharacters(DAVA::Scene* scene);
    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override;
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void OnClientConnected(const DAVA::Responder& responder);

private:
    DAVA::IServer* server;
    DAVA::Vector<const DAVA::Responder*> connectedResponders;

    DAVA::UnorderedSet<DAVA::Entity*> characters;
    DAVA::Entity* focusedCharacter;
    DAVA::CameraComponent* cameraComponent;
};
