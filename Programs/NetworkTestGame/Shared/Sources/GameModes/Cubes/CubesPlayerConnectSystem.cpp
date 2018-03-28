#include "GameModes/Cubes/CubesPlayerConnectSystem.h"

#include "GameModes/Cubes/CubesUtils.h"

#include "GameModes/Cubes/BigCubeComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(CubesPlayerConnectSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<CubesPlayerConnectSystem>::Begin()[M::Tags("gm_cubes", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &CubesPlayerConnectSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 25.1f)]
    .End();
}

CubesPlayerConnectSystem::CubesPlayerConnectSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentMask())
{
    networkServerConnectionsSingleComponent = scene->GetSingleComponent<DAVA::NetworkServerConnectionsSingleComponent>();
}

void CubesPlayerConnectSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (const DAVA::FastName& token : networkServerConnectionsSingleComponent->GetJustConnectedTokens())
    {
        CreatePlayer(token);
    }
}

void CubesPlayerConnectSystem::CreatePlayer(const DAVA::FastName& token) const
{
    using namespace DAVA;

    NetworkGameModeSingleComponent* networkGameModeSingleComponent = GetScene()->GetSingleComponent<NetworkGameModeSingleComponent>();
    NetworkPlayerID playerId = networkGameModeSingleComponent->GetNetworkPlayerID(token);
    Entity* playerEntity = networkGameModeSingleComponent->GetPlayerEnity(playerId);

    if (playerEntity == nullptr)
    {
        playerEntity = new Entity();

        BigCubeComponent* bigCubeComponent = new BigCubeComponent();
        bigCubeComponent->playerId = playerId;
        playerEntity->AddComponent(bigCubeComponent);

        playerEntity->GetComponent<TransformComponent>()->SetLocalTransform({ -0.5f, 15.0f, 5.f }, {}, Vector3::Zero + 1.f);

        GetScene()->AddNode(playerEntity);
    }
}
