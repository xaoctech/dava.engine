#pragma once

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/UnordererMap.h"
#include "Base/List.h"
#include "Entity/SceneSystem.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "Base/Vector.h"

#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"

#ifdef SERVER
#include "NetworkCore/Private/NetworkBuffer.h"
#include "NetworkCore/UDPTransport/UDPServer.h"
#else
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/Private/NetworkBucket.h"
#endif

#include <Base/BaseTypes.h>
#include <Base/UnordererMap.h>
#include <Base/List.h>
#include <Entity/SceneSystem.h>
#include <Math/Matrix4.h>
#include <Math/Vector.h>
#include <Base/Vector.h>

#include <memory>

namespace DAVA
{
class Entity;
class ActionsSingleComponent;

class NetworkInputSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkInputSystem, SceneSystem);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 timeElapsed) override;

#if !defined(SERVER)
    // This is workaround so that client input system worked at both beginning and the end of a frame
    // It's required to fill action's clientFrameId field
    // TODO: whole network input system should be discussed and refactored
    void ProcessFixedClientBegin(float32 timeElapsed);
#endif

    void PrepareForRemove() override{};

    NetworkInputSystem(Scene* scene);

#ifdef SERVER
    void OnConnect(const Responder& token);
    void OnReceive(const Responder& responder, const uint8* data, size_t size);
#else
    void SendLastBuckets();
    void OnConnect();
#endif

    static void PackDigitalActions(Scene* scene, uint64& packedActions, Entity* entity = nullptr, const UnorderedSet<FastName>* filter = nullptr);
    static Vector<FastName> UnpackDigitalActions(uint64 packedActions, Scene* scene);

    static uint64 PackAnalogActions(Scene* scene, uint64& packedActions, Entity* entity = nullptr, const UnorderedSet<FastName>* filter = nullptr);
    static AnalogActionsMap UnpackAnalogActions(uint64 packedActions,
                                                uint64 packedAnalogStates, Scene* scene);

private:
    static const uint8 DUPLICATE_INPUT_MARK = 0xFF;
    
#ifdef SERVER
    using NetworkInputBuffer = NetworkBuffer<NetworkInputComponent::Data>;
    UnorderedMap<Entity*, NetworkInputBuffer> entitiesToBuffers;
    UnorderedMap<FastName, UnorderedSet<Entity*>> tokensToEntities;
#else
    UnorderedSet<Entity*> entities;
    IClient* client;
#endif
};
}
