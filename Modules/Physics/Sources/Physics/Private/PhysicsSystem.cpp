#include "Physics/Private/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>
#include <FileSystem/KeyedArchive.h>

#include <physx/PxScene.h>
#include <PxShared/foundation/PxAllocatorCallback.h>
#include <PxShared/foundation/PxFoundation.h>

namespace DAVA
{
PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
{
    const KeyedArchive* options = Engine::Instance()->GetOptions();

    simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", 16 * 1024 * 512);
    DVASSERT((simulationBlockSize % (16 * 1024)) == 0);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
    physx::PxAllocatorCallback& allocator = physics->GetFoundation()->getAllocatorCallback();
    simulationBlock = allocator.allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = options->GetVector3("physics.gravity", Vector3(0, 0, -9.81f));
    sceneConfig.threadCount = options->GetUInt32("physics.threadCount", 2);
    physicsScene = physics->CreateScene(sceneConfig);
}

PhysicsSystem::~PhysicsSystem()
{
    DVASSERT(simulationBlock != nullptr);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
    physx::PxAllocatorCallback& allocator = physics->GetFoundation()->getAllocatorCallback();
    allocator.deallocate(simulationBlock);
    simulationBlock = nullptr;
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning && FetchResults(false))
    {
        isSimulationRunning = false;
    }

    if (isSimulationEnabled == false)
    {
        return;
    }

    if (isSimulationRunning == false)
    {
        physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);
        isSimulationRunning = true;
    }
}

void PhysicsSystem::SetSimulationEnabled(bool isEnabled)
{
    if (isSimulationEnabled != isEnabled)
    {
        if (isSimulationRunning == true)
        {
            DVASSERT(isSimulationEnabled == true);
            bool success = FetchResults(true);
            DVASSERT(success == true);
        }

        isSimulationEnabled = isEnabled;
    }
}

bool PhysicsSystem::IsSimulationEnabled() const
{
    return isSimulationEnabled;
}

bool PhysicsSystem::FetchResults(bool block)
{
    DVASSERT(isSimulationRunning);
    return physicsScene->fetchResults(block);
}

} // namespace DAVA
