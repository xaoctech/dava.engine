#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"
#include "Scene3D/Scene.h"
#include "Entity/SceneSystem.h"
#include "Entity/Component.h"
#include "Base/BaseTypes.h"

#include "Systems/GameVisibilitySystem.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Components/PlayerTankComponent.h"

using namespace DAVA;

DAVA_TESTCLASS (GameVisibilitySystemTest)
{
    DAVA_TEST (CheckAOI)
    {
        const uint8 ENTITIES_NUM = 3;
        ScopedPtr<Scene> scene(new Scene(0));
        NetworkVisibilitySingleComponent* visibilitySingleComponent = scene->GetSingletonComponent<NetworkVisibilitySingleComponent>();
        GameVisibilitySystem* visibilitySystem = new GameVisibilitySystem(scene);
        scene->AddSystem(visibilitySystem);

        ScopedPtr<Entity> player1(new Entity());
        ScopedPtr<Entity> player2(new Entity());
        const Vector<Entity*> entities = { nullptr, player1, player2 };
        TEST_VERIFY(ENTITIES_NUM == entities.size());
        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            Entity* entity = entities[playerID];
            NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent();
            replicationComponent->SetNetworkPlayerID(playerID);
            replicationComponent->SetNetworkID(NetworkID(playerID));
            replicationComponent->SetOwnerTeamID(playerID % 2);
            replicationComponent->SetEntityType(EntityType::VEHICLE);
            entity->AddComponent(replicationComponent);
            entity->AddComponent(new PlayerTankComponent());
            entity->AddComponent(new NetworkTransformComponent());
            scene->AddNode(entity);
        }

        float32 distance = 0;
        uint8 frequency = 1;
        while (distance < visibilitySystem->GetMaxAOI())
        {
            visibilitySystem->ProcessFixed(0.016f);
            TEST_VERIFY(frequency == visibilitySingleComponent->GetVisibility(1, player2));

            distance += visibilitySystem->GetPeriodIncreaseDistance() + 0.1f;
            Matrix4 trans = player1->GetLocalTransform();
            trans.SetTranslationVector(Vector3(distance, 0.f, 0.f));
            player1->SetLocalTransform(trans);
            ++frequency;
        }

        visibilitySystem->ProcessFixed(0.016f);
        TEST_VERIFY(distance > visibilitySystem->GetMaxAOI());
        TEST_VERIFY(0 == visibilitySingleComponent->GetVisibility(1, player2));
    }
};
