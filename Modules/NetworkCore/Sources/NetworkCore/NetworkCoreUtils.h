#pragma once

#include "NetworkTypes.h"
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Scene;
class SceneSystem;
class Entity;
class Component;

bool IsServer(Scene* scene);
bool IsServer(SceneSystem* sceneSystem);

bool IsClient(Scene* scene);
bool IsClient(SceneSystem* sceneSystem);

bool IsClientOwner(Scene* scene, const Entity* entity);
bool IsClientOwner(SceneSystem* sceneSystem, const Entity* entity);
bool IsClientOwner(const Entity* entity);

Entity* GetEntityWithNetworkId(Scene* scene, NetworkID networkId);

// TODO: --> move to NetworkInputUtils
Vector<ActionsSingleComponent::Actions>& GetCollectedActionsForClient(Scene* scene, const Entity* clientEntity);
void AddActionsForClient(Scene* scene, const Entity* clientEntity, ActionsSingleComponent::Actions&& actions);
void AddDigitalActionForClient(Scene* scene, const Entity* clientEntity, const FastName& action);
void AddAnalogActionForClient(Scene* scene, const Entity* clientEntity, const FastName& action, const Vector2& data);
// <--

struct NetworkCoreUtils
{
    static NetworkID GetEntityId(Entity* entity);
    static uint32 GetPlayerId(Entity* entity);

    static const uint32 ENET_DEFAULT_MTU_UNCOMPRESSED;
};

} // end namespace DAVA
