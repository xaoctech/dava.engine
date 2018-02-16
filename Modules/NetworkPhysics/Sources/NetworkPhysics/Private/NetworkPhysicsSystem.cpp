#include "NetworkPhysics/NetworkPhysicsSystem.h"
#include "NetworkPhysics/NetworkDynamicBodyComponent.h"
#include "NetworkPhysics/NetworkVehicleCarComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/VehicleChassisComponent.h>

#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

#include <physx/vehicle/PxVehicleDriveNW.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPhysicsSystem)
{
    ReflectionRegistrator<NetworkPhysicsSystem>::Begin()[M::Tags("network", "physics")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkPhysicsSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 0.1f)]
    .End();
}

NetworkPhysicsSystem::NetworkPhysicsSystem(Scene* scene)
    : DAVA::BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkDynamicBodyComponent>())
{
    dynamicBodies = scene->AquireComponentGroup<DynamicBodyComponent, DynamicBodyComponent>();
}

void NetworkPhysicsSystem::AddEntity(Entity* entity)
{
    NetworkDynamicBodyComponent* networkDynamicBodyComponent = entity->GetComponent<NetworkDynamicBodyComponent>();
    DVASSERT(networkDynamicBodyComponent != nullptr);

    networkDynamicBodyComponents.insert(networkDynamicBodyComponent);
}

void NetworkPhysicsSystem::RemoveEntity(Entity* entity)
{
    NetworkDynamicBodyComponent* networkDynamicBodyComponent = entity->GetComponent<NetworkDynamicBodyComponent>();
    DVASSERT(networkDynamicBodyComponent != nullptr);

    networkDynamicBodyComponents.erase(networkDynamicBodyComponent);
}

void NetworkPhysicsSystem::ProcessFixed(float32 dt)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkPhysicsSystem::ProcessFixed");

    if (networkDynamicBodyComponents.size() == 0)
    {
        return;
    }

    if (IsServer(GetScene()))
    {
        TransferDataToNetworkComponents();
    }
}

void NetworkPhysicsSystem::PrepareForRemove()
{
}

void NetworkPhysicsSystem::ReSimulationStart(DAVA::Entity* entity, uint32 frameId)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkPhysicsSystem::ReSimulationStart");

    DVASSERT(IsClient(GetScene()));

    NetworkDynamicBodyComponent* networkDynamicBodyComponent = entity->GetComponent<NetworkDynamicBodyComponent>();
    DVASSERT(networkDynamicBodyComponent != nullptr);
    DVASSERT(networkDynamicBodyComponents.find(networkDynamicBodyComponent) != networkDynamicBodyComponents.end());

    FreezeEverythingExceptEntity(entity);
    TransferDataFromNetworkComponent(networkDynamicBodyComponent);
}

void NetworkPhysicsSystem::Simulate(Entity* entity)
{
    PhysicsSystem* physicsSystem = GetScene()->GetSystem<PhysicsSystem>();
    DVASSERT(physicsSystem);

    physicsSystem->ProcessFixedSimulate(NetworkTimeSingleComponent::FrameDurationS);
    physicsSystem->ProcessFixedFetch(NetworkTimeSingleComponent::FrameDurationS);
}

void NetworkPhysicsSystem::ReSimulationEnd(Entity* entity)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkPhysicsSystem::ReSimulationEnd");

    UnfreezeEverything();
}

void NetworkPhysicsSystem::TransferDataToNetworkComponents()
{
    DAVA_PROFILER_CPU_SCOPE("NetworkPhysicsSystem::TransferDataToNetworkComponents");

    DVASSERT(IsServer(GetScene()));

    for (NetworkDynamicBodyComponent* networkDynamicBodyComponent : networkDynamicBodyComponents)
    {
        DVASSERT(networkDynamicBodyComponent != nullptr);

        Entity* entity = networkDynamicBodyComponent->GetEntity();
        DVASSERT(entity != nullptr);

        // Fill network dynamic body component

        DynamicBodyComponent* dynamicBodyComponent = entity->GetComponent<DynamicBodyComponent>();

        if (dynamicBodyComponent != nullptr)
        {
            networkDynamicBodyComponent->linearVelocity = dynamicBodyComponent->GetLinearVelocity();
            networkDynamicBodyComponent->angularVelocity = dynamicBodyComponent->GetAngularVelocity();

            // Fill network vehicle car component if exists

            NetworkVehicleCarComponent* networkVehicleCarComponent = entity->GetComponent<NetworkVehicleCarComponent>();
            if (networkVehicleCarComponent != nullptr)
            {
                VehicleCarComponent* vehicleCarComponent = entity->GetComponent<VehicleCarComponent>();
                DVASSERT(vehicleCarComponent != nullptr);

                physx::PxVehicleDrive* pxVehicleDrive = vehicleCarComponent->GetPxVehicle();
                physx::PxVehicleDriveNW* pxVehicleDriveNW = static_cast<physx::PxVehicleDriveNW*>(pxVehicleDrive);
                if (pxVehicleDriveNW != nullptr)
                {
                    uint32 numWheels = pxVehicleDriveNW->mWheelsSimData.getNbWheels();

                    networkVehicleCarComponent->numWheels = numWheels;
                    networkVehicleCarComponent->engineRotationSpeed = pxVehicleDriveNW->mDriveDynData.getEngineRotationSpeed();
                    networkVehicleCarComponent->gear = pxVehicleDriveNW->mDriveDynData.getCurrentGear();

                    for (uint32 i = 0; i < physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS; ++i)
                    {
                        networkVehicleCarComponent->analogInputStates[i] = pxVehicleDriveNW->mDriveDynData.getAnalogInput(i);
                    }

                    pxVehicleDriveNW->mWheelsDynData.getWheels4InternalJounces(networkVehicleCarComponent->jounces.data());

                    for (uint32 i = 0; i < numWheels; ++i)
                    {
                        pxVehicleDriveNW->mWheelsDynData.getWheelRotationSpeed(i, networkVehicleCarComponent->wheelsRotationSpeed[i], networkVehicleCarComponent->wheelsCorrectedRotationSpeed[i]);
                        networkVehicleCarComponent->wheelsRotationAngle[i] = pxVehicleDriveNW->mWheelsDynData.getWheelRotationAngle(i);
                    }

                    int32 wheelIndex = 0;
                    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
                    {
                        Entity* child = entity->GetChild(i);
                        DVASSERT(child != nullptr);

                        TransformComponent* childTransformComponent = child->GetComponent<TransformComponent>();

                        VehicleWheelComponent* wheelComponent = child->GetComponent<VehicleWheelComponent>();
                        if (wheelComponent != nullptr)
                        {
                            DVASSERT(static_cast<uint32>(wheelIndex) < numWheels);
                            networkVehicleCarComponent->wheelsOrientation[wheelIndex] = childTransformComponent->GetRotation();
                            networkVehicleCarComponent->wheelsPosition[wheelIndex] = childTransformComponent->GetPosition();

                            ++wheelIndex;
                        }

                        VehicleChassisComponent* chassisComponent = child->GetComponent<VehicleChassisComponent>();
                        if (chassisComponent != nullptr)
                        {
                            DVASSERT(wheelComponent == nullptr);

                            networkVehicleCarComponent->chassisOrientation = childTransformComponent->GetRotation();
                            networkVehicleCarComponent->chassisPosition = childTransformComponent->GetPosition();
                        }
                    }

                    DVASSERT(wheelIndex == numWheels);
                }
            }
        }
    }
}

void NetworkPhysicsSystem::TransferDataFromNetworkComponent(NetworkDynamicBodyComponent* networkDynamicBodyComponent)
{
    DAVA_PROFILER_CPU_SCOPE("NetworkPhysicsSystem::TransferDataFromNetworkComponents");

    DVASSERT(!IsServer(GetScene()));

    DVASSERT(networkDynamicBodyComponent != nullptr);

    Entity* entity = networkDynamicBodyComponent->GetEntity();
    DVASSERT(entity != nullptr);

    // Fill dynamic body component

    DynamicBodyComponent* dynamicBodyComponent = entity->GetComponent<DynamicBodyComponent>();

    if (dynamicBodyComponent != nullptr)
    {
        DVASSERT(dynamicBodyComponent != nullptr);

        dynamicBodyComponent->SetLinearVelocity(networkDynamicBodyComponent->linearVelocity);
        dynamicBodyComponent->SetAngularVelocity(networkDynamicBodyComponent->angularVelocity);

        // Fill vehicle car component if exists

        NetworkVehicleCarComponent* networkVehicleCarComponent = entity->GetComponent<NetworkVehicleCarComponent>();
        if (networkVehicleCarComponent != nullptr)
        {
            VehicleCarComponent* vehicleCarComponent = entity->GetComponent<VehicleCarComponent>();
            DVASSERT(vehicleCarComponent != nullptr);

            physx::PxVehicleDrive* pxVehicleDrive = vehicleCarComponent->GetPxVehicle();
            physx::PxVehicleDriveNW* pxVehicleDriveNW = static_cast<physx::PxVehicleDriveNW*>(pxVehicleDrive);
            DVASSERT(pxVehicleDriveNW != nullptr);

            pxVehicleDriveNW->mDriveDynData.setEngineRotationSpeed(networkVehicleCarComponent->engineRotationSpeed);
            pxVehicleDriveNW->mDriveDynData.setCurrentGear(networkVehicleCarComponent->gear);
            vehicleCarComponent->SetGear(static_cast<eVehicleGears>(networkVehicleCarComponent->gear));

            for (uint32 i = 0; i < physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS; ++i)
            {
                pxVehicleDriveNW->mDriveDynData.setAnalogInput(i, networkVehicleCarComponent->analogInputStates[i]);
            }

            pxVehicleDriveNW->mWheelsDynData.setWheels4InternalJounces(networkVehicleCarComponent->jounces.data());

            for (uint32 i = 0; i < networkVehicleCarComponent->numWheels; ++i)
            {
                pxVehicleDriveNW->mWheelsDynData.setWheelRotationSpeed(i, networkVehicleCarComponent->wheelsRotationSpeed[i], networkVehicleCarComponent->wheelsCorrectedRotationSpeed[i]);
                pxVehicleDriveNW->mWheelsDynData.setWheelRotationAngle(i, networkVehicleCarComponent->wheelsRotationAngle[i]);
            }

            if (networkVehicleCarComponent->numWheels > 0)
            {
                int32 wheelIndex = 0;
                for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
                {
                    Entity* child = entity->GetChild(i);
                    DVASSERT(child != nullptr);

                    TransformComponent* childTransformComponent = child->GetComponent<TransformComponent>();

                    VehicleWheelComponent* wheelComponent = child->GetComponent<VehicleWheelComponent>();
                    if (wheelComponent != nullptr)
                    {
                        DVASSERT(static_cast<uint32>(wheelIndex) < networkVehicleCarComponent->numWheels);

                        childTransformComponent->SetLocalTransform(networkVehicleCarComponent->wheelsPosition[wheelIndex], networkVehicleCarComponent->wheelsOrientation[wheelIndex], childTransformComponent->GetScale());

                        ++wheelIndex;
                    }

                    VehicleChassisComponent* chassisComponent = child->GetComponent<VehicleChassisComponent>();
                    if (chassisComponent != nullptr)
                    {
                        DVASSERT(wheelComponent == nullptr);

                        childTransformComponent->SetLocalTransform(networkVehicleCarComponent->chassisPosition, networkVehicleCarComponent->chassisOrientation, childTransformComponent->GetScale());
                    }
                }

                DVASSERT(wheelIndex == networkVehicleCarComponent->numWheels);
            }
        }
    }
}

void NetworkPhysicsSystem::FreezeEverythingExceptEntity(Entity* entity)
{
    DVASSERT(frozenDynamicBodiesParams.size() == 0);

    PhysicsSystem* physicsSystem = GetScene()->GetSystem<PhysicsSystem>();
    DVASSERT(physicsSystem);

    for (DynamicBodyComponent* body : dynamicBodies->components)
    {
        Entity* currentEntity = body->GetEntity();
        DVASSERT(currentEntity != nullptr);

        if (currentEntity != entity)
        {
            if (body->GetIsKinematic() == false)
            {
                frozenDynamicBodiesParams[body] = std::make_tuple(body->GetLinearVelocity(), body->GetAngularVelocity());
                body->SetIsKinematic(true);
            }
        }
    }
}

void NetworkPhysicsSystem::UnfreezeEverything()
{
    for (auto& frozenDynamicBodyKvp : frozenDynamicBodiesParams)
    {
        DynamicBodyComponent* dynamicBody = frozenDynamicBodyKvp.first;
        DVASSERT(dynamicBody != nullptr);

        auto& params = frozenDynamicBodyKvp.second;

        dynamicBody->SetIsKinematic(false);
        dynamicBody->SetLinearVelocity(std::get<0>(params));
        dynamicBody->SetAngularVelocity(std::get<1>(params));
    }

    frozenDynamicBodiesParams.clear();
}

void NetworkPhysicsSystem::LogVehicleCar(VehicleCarComponent* carComponent, String const& header)
{
    using namespace physx;

    Entity* entity = carComponent->GetEntity();
    DVASSERT(entity != nullptr);

    DynamicBodyComponent* dynamicBodyComponent = entity->GetComponent<DynamicBodyComponent>();
    PxVehicleDriveNW* pxVehicle = static_cast<PxVehicleDriveNW*>(carComponent->GetPxVehicle());
    if (pxVehicle == nullptr)
        return;
    PxRigidDynamic* actor = pxVehicle->getRigidDynamicActor();

    Logger::Info("VEHICLE STATE %s (frame: %d) =============", header.c_str(), GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>()->GetFrameId());
    Logger::Info("Actor global position: (%f, %f, %f), global rotation: (%f, %f, %f, %f)", actor->getGlobalPose().p.x, actor->getGlobalPose().p.y, actor->getGlobalPose().p.z, actor->getGlobalPose().q.x, actor->getGlobalPose().q.y, actor->getGlobalPose().q.z, actor->getGlobalPose().q.w);
    Logger::Info("Actor mass: %f", actor->getMass());

    static const uint32 NUM_MAX_SHAPES = 5;
    PxShape* shapes[NUM_MAX_SHAPES];
    actor->getShapes(shapes, NUM_MAX_SHAPES);
    for (uint32 i = 0; i < NUM_MAX_SHAPES; ++i)
    {
        PxShape* shape = shapes[i];
        if (shape != nullptr)
        {
            Logger::Info("Shape %d local position: (%f, %f, %f), global rotation: (%f, %f, %f, %f)", i, shape->getLocalPose().p.x, shape->getLocalPose().p.y, shape->getLocalPose().p.z, shape->getLocalPose().q.x, shape->getLocalPose().q.y, shape->getLocalPose().q.z, shape->getLocalPose().q.w);
        }
    }

    Logger::Info("Linear velocity: %f, %f, %f", actor->getLinearVelocity().x, actor->getLinearVelocity().y, actor->getLinearVelocity().z);
    Logger::Info("Angular velocity: %f, %f, %f", actor->getAngularVelocity().x, actor->getAngularVelocity().y, actor->getAngularVelocity().z);
    Logger::Info("Engine speed: %f", pxVehicle->mDriveDynData.getEngineRotationSpeed());
    Logger::Info("Gear: %u", pxVehicle->mDriveDynData.getCurrentGear());
    Logger::Info("Gear change: %u", pxVehicle->mDriveDynData.getGearChange());
    Logger::Info("Gear down: %u", (uint32)pxVehicle->mDriveDynData.getGearDown());
    Logger::Info("Gear up: %u", (uint32)pxVehicle->mDriveDynData.getGearUp());
    Logger::Info("Target gear: %u", pxVehicle->mDriveDynData.getTargetGear());
    Logger::Info("Linear damping = %f, angular damping = %f", actor->getLinearDamping(), actor->getAngularDamping());
    Logger::Info("CMass: (%f, %f, %f), (%f, %f, %f %f)", actor->getCMassLocalPose().p.x, actor->getCMassLocalPose().p.y, actor->getCMassLocalPose().p.z, actor->getCMassLocalPose().q.x, actor->getCMassLocalPose().q.y, actor->getCMassLocalPose().q.z, actor->getCMassLocalPose().q.w);
    Logger::Info("AABB: min (%f, %f, %f), max (%f, %f, %f)", actor->getWorldBounds().minimum.x, actor->getWorldBounds().minimum.y, actor->getWorldBounds().minimum.z, actor->getWorldBounds().maximum.x, actor->getWorldBounds().maximum.y, actor->getWorldBounds().maximum.z);
    Logger::Info("Computed forward speed: %f", pxVehicle->computeForwardSpeed());
    Logger::Info("Computed sideway speed: %f", pxVehicle->computeSidewaysSpeed());
    Logger::Info("Autobox switch time: %f", pxVehicle->mDriveDynData.getAutoBoxSwitchTime());
    Logger::Info("Gear switch time: %f", pxVehicle->mDriveDynData.getGearSwitchTime());
    Logger::Info("Use autogears: %u", (uint32)pxVehicle->mDriveDynData.getUseAutoGears());
    Logger::Info("Num inputs: %u", pxVehicle->mDriveDynData.getNbAnalogInput());

    float32 jounces[4];
    pxVehicle->mWheelsDynData.getWheels4InternalJounces(jounces);

    for (uint32 i = 0; i < 4; ++i)
    {
        float rotationSpeed;
        float correctedRotationSpeed;
        pxVehicle->mWheelsDynData.getWheelRotationSpeed(i, rotationSpeed, correctedRotationSpeed);

        Logger::Info("Wheel %u, rotation speed %f, corrected rotation speed %f", i, rotationSpeed, correctedRotationSpeed);
        Logger::Info("Wheel %u, rotation angle %f", i, pxVehicle->mWheelsDynData.getWheelRotationAngle(i));
    }

    for (uint32 i = 0; i < 4; ++i)
    {
        Logger::Info("Jounce %u, value %f", i, jounces[i]);
    }

    Logger::Info("Component input: acceleration = %f", carComponent->GetAnalogAcceleration());
    Logger::Info("Component input: steer = %f", carComponent->GetAnalogSteer());
    Logger::Info("Component input: brake = %f", carComponent->GetAnalogBrake());
    Logger::Info("PxVehicle analog input: steer = %f, %f", pxVehicle->mDriveDynData.getAnalogInput(PxVehicleDriveNWControl::eANALOG_INPUT_STEER_LEFT), pxVehicle->mDriveDynData.getAnalogInput(PxVehicleDriveNWControl::eANALOG_INPUT_STEER_RIGHT));
    Logger::Info("PxVehicle analog input: accel = %f, brake = %f, handbrake = %f", pxVehicle->mDriveDynData.getAnalogInput(PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL), pxVehicle->mDriveDynData.getAnalogInput(PxVehicleDriveNWControl::eANALOG_INPUT_BRAKE), pxVehicle->mDriveDynData.getAnalogInput(PxVehicleDriveNWControl::eANALOG_INPUT_HANDBRAKE));

    Logger::Info("===========================");
}
}
