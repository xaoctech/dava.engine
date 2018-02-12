#pragma once

#include <Entity/SceneSystem.h>
#include <Entity/SingletonComponent.h>
#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

/* I want to do mini "hello world" example
   to create string on user click into window
   and share string value
*/

class HelloWorldComponent : public DAVA::SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(HelloWorldComponent, DAVA::SingletonComponent);
    DAVA::String helloMsg;
};

class Game01HelloWorld : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(Game01HelloWorld, DAVA::INetworkInputSimulationSystem);

    void CreateEntityAfterClientConnected(DAVA::Scene* scene);
    explicit Game01HelloWorld(DAVA::Scene* scene);
    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;

    void ApplyDigitalActions(DAVA::Entity* entity,
                             const DAVA::Vector<DAVA::FastName>& actions,
                             DAVA::uint32 clientFrameId,
                             DAVA::float32 duration) const override;
};
