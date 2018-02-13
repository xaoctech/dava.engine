#include "UnitTests/UnitTests.h"

#include "Engine/Engine.h"
#include "Scene3D/Scene.h"
#include "Entity/SceneSystem.h"
#include "Entity/Component.h"
#include "Base/BaseTypes.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/Scene3D/Systems/NetworkVisibilitySystem.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkVisibilitySingleComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"

using namespace DAVA;

DAVA_TESTCLASS (NetworkVisibilitySystemTest)
{
    DAVA_TEST (CheckVisibilityStatus)
    {
        const uint8 VIS_FREQ = 1;
        const uint8 INVIS_FREQ = 0;
        const uint8 ENTITIES_NUM = 3;

        ScopedPtr<Scene> scene(new Scene(0));

        NetworkVisibilitySystem* visibilitySystem = new NetworkVisibilitySystem(scene);
        scene->AddSystem(visibilitySystem);

        NetworkVisibilitySingleComponent* visibilitySingleComponent = scene->GetSingletonComponent<NetworkVisibilitySingleComponent>();

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
            entity->AddComponent(new NetworkPlayerComponent());
            scene->AddNode(entity);
        }

        /*
        *   Case1: Player1 and Player2 see each other.
        */
        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            Entity* entity = entities[playerID];
            const uint8 otherPlayerID = ENTITIES_NUM - playerID;
            Entity* otherPlayer = entities[otherPlayerID];
            visibilitySingleComponent->SetVisibility(playerID, otherPlayer, VIS_FREQ);
            TEST_VERIFY(VIS_FREQ == visibilitySingleComponent->GetVisibility(playerID, entities[otherPlayerID]));

            const NetworkVisibilitySingleComponent::EntityToFrequency& entityToFrequency =
            visibilitySingleComponent->GetVisibleEntities(playerID);
            TEST_VERIFY(entityToFrequency.size() == 1);
            TEST_VERIFY(entityToFrequency.find(otherPlayer) != entityToFrequency.end());

            const Vector<NetworkID>& entityIDs = visibilitySingleComponent->GetVisibleNetworkIDs(playerID);
            TEST_VERIFY(entityIDs.size() == 1);
            TEST_VERIFY(entityIDs[0] == NetworkID(otherPlayerID));
        }

        visibilitySystem->ProcessFixed(0.016f);

        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            const UnorderedSet<const Entity*>& addedEntities = visibilitySingleComponent->GetAddedEntities(playerID);
            const uint8 otherPlayerID = ENTITIES_NUM - playerID;
            Entity* otherPlayer = entities[otherPlayerID];
            TEST_VERIFY(addedEntities.find(otherPlayer) != addedEntities.end());

            NetworkPlayerComponent* networkPlayerComponent = entities[playerID]->GetComponent<NetworkPlayerComponent>();
            const FixedVector<NetworkID>& visibleEntityIds = networkPlayerComponent->visibleEntityIds;
            TEST_VERIFY(visibleEntityIds.size() == 1);
            TEST_VERIFY(visibleEntityIds[0] == NetworkID(otherPlayerID));
        }
        visibilitySingleComponent->Clear();

        /*
         *  Case2: Player1 and Player2 see nothing.
         */
        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            Entity* entity = entities[playerID];
            const uint8 otherPlayerID = ENTITIES_NUM - playerID;
            Entity* otherPlayer = entities[otherPlayerID];
            visibilitySingleComponent->SetVisibility(playerID, entities[otherPlayerID], INVIS_FREQ);
            TEST_VERIFY(INVIS_FREQ == visibilitySingleComponent->GetVisibility(playerID, entities[otherPlayerID]));

            const NetworkVisibilitySingleComponent::EntityToFrequency& entityToFrequency =
            visibilitySingleComponent->GetVisibleEntities(playerID);

            TEST_VERIFY(entityToFrequency.size() == 0);
            TEST_VERIFY(entityToFrequency.find(otherPlayer) == entityToFrequency.end());

            const Vector<NetworkID>& entityIDs = visibilitySingleComponent->GetVisibleNetworkIDs(playerID);
            TEST_VERIFY(entityIDs.size() == 0);
        }

        visibilitySystem->ProcessFixed(0.016f);

        for (uint8 playerID = 1; playerID < ENTITIES_NUM; ++playerID)
        {
            const UnorderedSet<const Entity*>& addedEntities = visibilitySingleComponent->GetAddedEntities(playerID);
            const uint8 otherPlayerID = ENTITIES_NUM - playerID;
            Entity* otherPlayer = entities[otherPlayerID];
            TEST_VERIFY(addedEntities.find(otherPlayer) == addedEntities.end());
            TEST_VERIFY(0 == entities[playerID]->GetComponent<NetworkPlayerComponent>()->visibleEntityIds.size());
        }
        visibilitySingleComponent->Clear();

        /*
         *  Case3: Player1 looks Player2, but Player2 does not look Player1.
         */
        visibilitySingleComponent->SetVisibility(1, player2, VIS_FREQ);
        TEST_VERIFY(VIS_FREQ == visibilitySingleComponent->GetVisibility(1, player2));
        TEST_VERIFY(INVIS_FREQ == visibilitySingleComponent->GetVisibility(2, player1));

        const NetworkVisibilitySingleComponent::EntityToFrequency& entityToFrequency1 =
        visibilitySingleComponent->GetVisibleEntities(1);

        TEST_VERIFY(entityToFrequency1.size() == 1);
        TEST_VERIFY(entityToFrequency1.find(entities[2]) != entityToFrequency1.end());

        const Vector<NetworkID>& entityIDs1 = visibilitySingleComponent->GetVisibleNetworkIDs(1);
        TEST_VERIFY(entityIDs1.size() == 1);
        TEST_VERIFY(entityIDs1[0] == NetworkID(2));

        const NetworkVisibilitySingleComponent::EntityToFrequency& entityToFrequency2 =
        visibilitySingleComponent->GetVisibleEntities(2);

        TEST_VERIFY(entityToFrequency2.size() == 0);
        TEST_VERIFY(entityToFrequency2.find(entities[1]) == entityToFrequency2.end());

        const Vector<NetworkID>& entityIDs2 = visibilitySingleComponent->GetVisibleNetworkIDs(2);
        TEST_VERIFY(entityIDs2.size() == 0);

        visibilitySystem->ProcessFixed(0.016f);

        const UnorderedSet<const Entity*>& addedEntities1 = visibilitySingleComponent->GetAddedEntities(1);
        TEST_VERIFY(addedEntities1.find(entities[2]) != addedEntities1.end());
        TEST_VERIFY(1 == player1->GetComponent<NetworkPlayerComponent>()->visibleEntityIds.size());

        const UnorderedSet<const Entity*>& addedEntities2 = visibilitySingleComponent->GetAddedEntities(2);
        TEST_VERIFY(addedEntities2.find(entities[1]) == addedEntities2.end());
        TEST_VERIFY(0 == player2->GetComponent<NetworkPlayerComponent>()->visibleEntityIds.size());

        visibilitySingleComponent->Clear();
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__
