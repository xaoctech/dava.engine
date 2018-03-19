#include "Physics/Vehicles/PhysicsVehiclesSubsystem.h"

#include "Physics/PhysicsModule.h"
#include "Physics/Core/PhysicsUtils.h"
#include "Physics/Core/PhysicsComponent.h"
#include "Physics/Core/BoxShapeComponent.h"
#include "Physics/Core/ConvexHullShapeComponent.h"
#include "Physics/Core/DynamicBodyComponent.h"
#include "Physics/Vehicles/VehicleCarComponent.h"
#include "Physics/Vehicles/VehicleTankComponent.h"
#include "Physics/Vehicles/VehicleChassisComponent.h"
#include "Physics/Vehicles/VehicleWheelComponent.h"
#include "Physics/Core/DynamicBodyComponent.h"
#include "Physics/Core/Private/PhysicsMath.h"

#include <Debug/ProfilerCPU.h>
#include <Engine/Engine.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SkeletonComponent.h>
#include <Logger/Logger.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>

#include <physx/PxActor.h>
#include <physx/PxScene.h>
#include <physx/PxBatchQueryDesc.h>
#include <physx/vehicle/PxVehicleUtil.h>
#include <physx/vehicle/PxVehicleTireFriction.h>
#include <physx/vehicle/PxVehicleUpdate.h>
#include <PxShared/foundation/PxAllocatorCallback.h>

#include <algorithm>

#define PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG(...) // Logger::Info(__VA_ARGS__)

namespace DAVA
{
const uint32 MAX_VEHICLES_COUNT = 100;

const uint32 DRIVABLE_SURFACE_FILTER = 0xffff0000;
const uint32 UNDRIVABLE_SURFACE_FILTER = 0x0000ffff;

const uint32 SURFACE_TYPE_NORMAL = 0;

const uint32 TIRE_TYPE_NORMAL = 0;

namespace PhysicsVehicleSubsystemDetail
{
// Data structure for quick setup of scene queries for suspension queries
class VehicleSceneQueryData
{
public:
    VehicleSceneQueryData();
    ~VehicleSceneQueryData();

    // Allocate scene query data for up to maxNumVehicles and up to maxNumWheelsPerVehicle with numVehiclesInBatch per batch query
    static VehicleSceneQueryData* Allocate
    (const physx::PxU32 maxNumVehicles, const physx::PxU32 maxNumWheelsPerVehicle, const physx::PxU32 maxNumHitPointsPerWheel, const physx::PxU32 numVehiclesInBatch,
     physx::PxBatchQueryPreFilterShader preFilterShader, physx::PxBatchQueryPostFilterShader postFilterShader,
     physx::PxAllocatorCallback& allocator);

    // Free allocated buffers
    void Free(physx::PxAllocatorCallback& allocator);

    // Create a PxBatchQuery instance that will be used for a single specified batch
    static physx::PxBatchQuery* SetUpBatchedSceneQuery(const physx::PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene);

    // Return an array of scene query results for a single specified batch
    physx::PxRaycastQueryResult* GetRaycastQueryResultBuffer(const physx::PxU32 batchId);

    // Return an array of scene query results for a single specified batch
    physx::PxSweepQueryResult* GetSweepQueryResultBuffer(const physx::PxU32 batchId);

    // Get the number of scene query results that have been allocated for a single batch
    physx::PxU32 GetQueryResultBufferSize() const;

private:
    // Number of queries per batch
    physx::PxU32 numQueriesPerBatch;

    // Number of hit results per query
    physx::PxU32 numHitResultsPerQuery;

    // One result for each wheel
    physx::PxRaycastQueryResult* raycastResults;
    physx::PxSweepQueryResult* sweepResults;

    // One hit for each wheel
    physx::PxRaycastHit* raycastHitBuffer;
    physx::PxSweepHit* sweepHitBuffer;

    // Filter shader used to filter drivable and non-drivable surfaces
    physx::PxBatchQueryPreFilterShader preFilterShader;

    // Filter shader used to reject hit shapes that initially overlap sweeps
    physx::PxBatchQueryPostFilterShader postFilterShader;
};

VehicleSceneQueryData::VehicleSceneQueryData()
    : numQueriesPerBatch(0)
    , numHitResultsPerQuery(0)
    , raycastResults(nullptr)
    , sweepResults(nullptr)
    , raycastHitBuffer(nullptr)
    , sweepHitBuffer(nullptr)
    , preFilterShader(nullptr)
    , postFilterShader(nullptr)
{
}

VehicleSceneQueryData::~VehicleSceneQueryData()
{
}

VehicleSceneQueryData* VehicleSceneQueryData::Allocate
(const physx::PxU32 maxNumVehicles, const physx::PxU32 maxNumWheelsPerVehicle, const physx::PxU32 maxNumHitPointsPerWheel, const physx::PxU32 numVehiclesInBatch,
 physx::PxBatchQueryPreFilterShader preFilterShader, physx::PxBatchQueryPostFilterShader postFilterShader,
 physx::PxAllocatorCallback& allocator)
{
    using namespace physx;

    const PxU32 sqDataSize = ((sizeof(VehicleSceneQueryData) + 15) & ~15);

    const PxU32 maxNumWheels = maxNumVehicles * maxNumWheelsPerVehicle;
    const PxU32 raycastResultSize = ((sizeof(PxRaycastQueryResult) * maxNumWheels + 15) & ~15);
    const PxU32 sweepResultSize = ((sizeof(PxSweepQueryResult) * maxNumWheels + 15) & ~15);

    const PxU32 maxNumHitPoints = maxNumWheels * maxNumHitPointsPerWheel;
    const PxU32 raycastHitSize = ((sizeof(PxRaycastHit) * maxNumHitPoints + 15) & ~15);
    const PxU32 sweepHitSize = ((sizeof(PxSweepHit) * maxNumHitPoints + 15) & ~15);

    const PxU32 size = sqDataSize + raycastResultSize + raycastHitSize + sweepResultSize + sweepHitSize;
    PxU8* buffer = static_cast<PxU8*>(allocator.allocate(size, "VehicleSceneQueryData", __FILE__, __LINE__));

    VehicleSceneQueryData* sqData = new (buffer) VehicleSceneQueryData();
    sqData->numQueriesPerBatch = numVehiclesInBatch * maxNumWheelsPerVehicle;
    sqData->numHitResultsPerQuery = maxNumHitPointsPerWheel;
    buffer += sqDataSize;

    sqData->raycastResults = reinterpret_cast<PxRaycastQueryResult*>(buffer);
    buffer += raycastResultSize;

    sqData->raycastHitBuffer = reinterpret_cast<PxRaycastHit*>(buffer);
    buffer += raycastHitSize;

    sqData->sweepResults = reinterpret_cast<PxSweepQueryResult*>(buffer);
    buffer += sweepResultSize;

    sqData->sweepHitBuffer = reinterpret_cast<PxSweepHit*>(buffer);
    buffer += sweepHitSize;

    for (PxU32 i = 0; i < maxNumWheels; i++)
    {
        new (sqData->raycastResults + i) PxRaycastQueryResult();
        new (sqData->sweepResults + i) PxSweepQueryResult();
    }

    for (PxU32 i = 0; i < maxNumHitPoints; i++)
    {
        new (sqData->raycastHitBuffer + i) PxRaycastHit();
        new (sqData->sweepHitBuffer + i) PxSweepHit();
    }

    sqData->preFilterShader = preFilterShader;
    sqData->postFilterShader = postFilterShader;

    return sqData;
}

void VehicleSceneQueryData::Free(physx::PxAllocatorCallback& allocator)
{
    allocator.deallocate(this);
}

physx::PxBatchQuery* VehicleSceneQueryData::SetUpBatchedSceneQuery(const physx::PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene)
{
    using namespace physx;

    const PxU32 maxNumQueriesInBatch = vehicleSceneQueryData.numQueriesPerBatch;
    const PxU32 maxNumHitResultsInBatch = vehicleSceneQueryData.numQueriesPerBatch * vehicleSceneQueryData.numHitResultsPerQuery;

    PxBatchQueryDesc sqDesc(maxNumQueriesInBatch, maxNumQueriesInBatch, 0);

    sqDesc.queryMemory.userRaycastResultBuffer = vehicleSceneQueryData.raycastResults + batchId * maxNumQueriesInBatch;
    sqDesc.queryMemory.userRaycastTouchBuffer = vehicleSceneQueryData.raycastHitBuffer + batchId * maxNumHitResultsInBatch;
    sqDesc.queryMemory.raycastTouchBufferSize = maxNumHitResultsInBatch;

    sqDesc.queryMemory.userSweepResultBuffer = vehicleSceneQueryData.sweepResults + batchId * maxNumQueriesInBatch;
    sqDesc.queryMemory.userSweepTouchBuffer = vehicleSceneQueryData.sweepHitBuffer + batchId * maxNumHitResultsInBatch;
    sqDesc.queryMemory.sweepTouchBufferSize = maxNumHitResultsInBatch;

    sqDesc.preFilterShader = vehicleSceneQueryData.preFilterShader;

    sqDesc.postFilterShader = vehicleSceneQueryData.postFilterShader;

    return scene->createBatchQuery(sqDesc);
}

physx::PxRaycastQueryResult* VehicleSceneQueryData::GetRaycastQueryResultBuffer(const physx::PxU32 batchId)
{
    return (raycastResults + batchId * numQueriesPerBatch);
}

physx::PxSweepQueryResult* VehicleSceneQueryData::GetSweepQueryResultBuffer(const physx::PxU32 batchId)
{
    return (sweepResults + batchId * numQueriesPerBatch);
}

physx::PxU32 VehicleSceneQueryData::GetQueryResultBufferSize() const
{
    return numQueriesPerBatch;
}

physx::PxVehicleDrivableSurfaceToTireFrictionPairs* CreateFrictionPairs(const physx::PxMaterial* defaultMaterial)
{
    using namespace physx;

    PxVehicleDrivableSurfaceType surfaceTypes[1];
    surfaceTypes[0].mType = SURFACE_TYPE_NORMAL;

    const PxMaterial* surfaceMaterials[1];
    surfaceMaterials[0] = defaultMaterial;

    PxVehicleDrivableSurfaceToTireFrictionPairs* surfaceTirePairs =
    PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(1, 1);
    surfaceTirePairs->setup(1, 1, surfaceMaterials, surfaceTypes);
    surfaceTirePairs->setTypePairFriction(0, 0, 1.0f);

    return surfaceTirePairs;
}

physx::PxQueryHitType::Enum WheelSceneQueryPreFilterBlocking(physx::PxFilterData filterData0, physx::PxFilterData filterData1, const void* constantBlock, physx::PxU32 constantBlockSize, physx::PxHitFlags& queryFlags)
{
    using namespace physx;

    // filterData0 is the vehicle suspension query
    // filterData1 is the shape potentially hit by the query
    PX_UNUSED(filterData0);
    PX_UNUSED(constantBlock);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(queryFlags);
    PxQueryHitType::Enum result = ((0 == (filterData1.word3 & DRIVABLE_SURFACE_FILTER)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);

    return result;
}

VehicleComponent* GetVehicleComponentFromEntity(Entity* entity)
{
    DVASSERT(entity != nullptr);

    VehicleComponent* vehicle = static_cast<VehicleComponent*>(entity->GetComponent<VehicleCarComponent>());
    if (vehicle == nullptr)
    {
        vehicle = static_cast<VehicleComponent*>(entity->GetComponent<VehicleTankComponent>());
    }

    return vehicle;
}
} // namespace PhysicsVehicleSubsystemDetail

PhysicsVehiclesSubsystem::PhysicsVehiclesSubsystem(Scene* scene, physx::PxScene* pxScene)
    : scene(scene)
    , pxScene(pxScene)
{
    DVASSERT(pxScene != nullptr);
    DVASSERT(scene != nullptr);

    using namespace physx;
    using namespace PhysicsVehicleSubsystemDetail;

    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(physics != nullptr);

    PxAllocatorCallback* allocator = physics->GetAllocator();
    DVASSERT(allocator != nullptr);

    vehicleSceneQueryData = VehicleSceneQueryData::Allocate(MAX_VEHICLES_COUNT, PX_MAX_NB_WHEELS, 1, MAX_VEHICLES_COUNT, WheelSceneQueryPreFilterBlocking, NULL, *allocator);
    batchQuery = VehicleSceneQueryData::SetUpBatchedSceneQuery(0, *vehicleSceneQueryData, pxScene);
    frictionPairs = CreateFrictionPairs(physics->GetMaterial(FastName()));

    cars = scene->AquireComponentGroup<VehicleCarComponent, VehicleCarComponent>();
    tanks = scene->AquireComponentGroup<VehicleTankComponent, VehicleTankComponent>();
    dynamicBodies = scene->AquireComponentGroup<DynamicBodyComponent, DynamicBodyComponent>();

    cars->onComponentRemoved->Connect(this, &PhysicsVehiclesSubsystem::OnCarComponentRemoved);
    tanks->onComponentRemoved->Connect(this, &PhysicsVehiclesSubsystem::OnTankComponentRemoved);
    dynamicBodies->onComponentRemoved->Connect(this, &PhysicsVehiclesSubsystem::OnDynamicBodyRemoved);
}

PhysicsVehiclesSubsystem::~PhysicsVehiclesSubsystem()
{
    using namespace physx;

    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(physics != nullptr);

    PxAllocatorCallback* allocator = physics->GetAllocator();
    DVASSERT(allocator != nullptr);

    vehicleSceneQueryData->Free(*allocator);
    batchQuery->release();
    frictionPairs->release();
}

void PhysicsVehiclesSubsystem::OnCarComponentRemoved(VehicleCarComponent* carComponent)
{
    DeinitVehicleComponent(carComponent);
}

void PhysicsVehiclesSubsystem::OnTankComponentRemoved(VehicleTankComponent* tankComponent)
{
    DeinitVehicleComponent(tankComponent);
}

void PhysicsVehiclesSubsystem::OnDynamicBodyRemoved(DynamicBodyComponent* body)
{
    Entity* entity = body->GetEntity();
    DVASSERT(entity != nullptr);

    VehicleComponent* vehicleComponent = PhysicsVehicleSubsystemDetail::GetVehicleComponentFromEntity(entity);
    if (vehicleComponent != nullptr)
    {
        DeinitVehicleComponent(vehicleComponent);
    }
}

void PhysicsVehiclesSubsystem::DeinitVehicleComponent(VehicleComponent* vehicleComponent)
{
    DVASSERT(vehicleComponent != nullptr);

    if (vehicleComponent->vehicle != nullptr)
    {
        vehicleComponent->vehicle->release();
        vehicleComponent->vehicle = nullptr;
    }
}

void PhysicsVehiclesSubsystem::ProcessFixed(float32 timeElapsed)
{
    using namespace physx;
    using namespace PhysicsVehicleSubsystemDetail;

    DAVA_PROFILER_CPU_SCOPE("PhysicsVehiclesSubsystem::ProcessFixed");

    static PxVehicleWheels* physxVehicles[MAX_VEHICLES_COUNT];
    static PxVehicleWheelQueryResult vehicleQueryResults[MAX_VEHICLES_COUNT];
    static PxWheelQueryResult wheelQueryResults[MAX_VEHICLES_COUNT][PX_MAX_NB_WHEELS];

    uint32 vehicleCount = 0;

    auto addVehicleToSimulation = [&vehicleCount](VehicleComponent* vehicleComponent)
    {
        if (vehicleComponent->vehicle != nullptr)
        {
            // Do not simulate vehicles marked as kinematic
            if (vehicleComponent->vehicle->getRigidDynamicActor()->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC) == true)
            {
                return;
            }

            physxVehicles[vehicleCount] = vehicleComponent->vehicle;
            vehicleQueryResults[vehicleCount].nbWheelQueryResults = physxVehicles[vehicleCount]->mWheelsSimData.getNbWheels();
            vehicleQueryResults[vehicleCount].wheelQueryResults = wheelQueryResults[vehicleCount];

            ++vehicleCount;

            DVASSERT(vehicleCount <= MAX_VEHICLES_COUNT);
        }
    };

    for (VehicleCarComponent* carComponent : cars->components)
    {
        if (carComponent->vehicle == nullptr)
        {
            TryRecreateCarVehicle(carComponent);
        }

        addVehicleToSimulation(carComponent);
    }

    for (VehicleTankComponent* tankComponent : tanks->components)
    {
        if (tankComponent->vehicle == nullptr)
        {
            TryRecreateTankVehicle(static_cast<VehicleTankComponent*>(tankComponent));
        }

        addVehicleToSimulation(tankComponent);
    }

    // Raycasts
    PxRaycastQueryResult* raycastResults = vehicleSceneQueryData->GetRaycastQueryResultBuffer(0);
    const PxU32 raycastResultsSize = vehicleSceneQueryData->GetQueryResultBufferSize();
    PxVehicleSuspensionRaycasts(batchQuery, vehicleCount, physxVehicles, raycastResultsSize, raycastResults);

    // Vehicle update
    const PxVec3 grav = pxScene->getGravity();
    PxVehicleUpdates(timeElapsed, grav, *frictionPairs, vehicleCount, physxVehicles, vehicleQueryResults);

    // Process input
    for (size_t i = 0; i < vehicleCount; ++i)
    {
        PxVehicleWheels* physxVehicle = physxVehicles[i];

        PhysicsComponent* physicsComponent = PhysicsComponent::GetComponent(physxVehicle->getRigidDynamicActor());
        DVASSERT(physicsComponent != nullptr);

        Entity* entity = physicsComponent->GetEntity();
        DVASSERT(entity != nullptr);

        VehicleComponent* vehicleComponent = PhysicsVehicleSubsystemDetail::GetVehicleComponentFromEntity(entity);
        DVASSERT(vehicleComponent != nullptr);

        if (vehicleComponent->GetType()->Is<VehicleCarComponent>())
        {
            VehicleCarComponent* car = static_cast<VehicleCarComponent*>(vehicleComponent);
            PxVehicleDriveNW* physxCar = static_cast<PxVehicleDriveNW*>(car->vehicle);

            PxVehiclePadSmoothingData smoothingData =
            {
              {
              6.0f, //rise rate eANALOG_INPUT_ACCEL
              6.0f, //rise rate eANALOG_INPUT_BRAKE
              6.0f, //rise rate eANALOG_INPUT_HANDBRAKE
              2.5f, //rise rate eANALOG_INPUT_STEER_LEFT
              2.5f, //rise rate eANALOG_INPUT_STEER_RIGHT
              },
              {
              10.0f, //fall rate eANALOG_INPUT_ACCEL
              10.0f, //fall rate eANALOG_INPUT_BRAKE
              10.0f, //fall rate eANALOG_INPUT_HANDBRAKE
              5.0f, //fall rate eANALOG_INPUT_STEER_LEFT
              5.0f //fall rate eANALOG_INPUT_STEER_RIGHT
              }
            };

            static PxF32 steerVsForwardSpeedData[2 * 8] =
            {
              0.0f, 0.75f,
              5.0f, 0.75f,
              30.0f, 0.125f,
              120.0f, 0.1f,
              PX_MAX_F32, PX_MAX_F32,
              PX_MAX_F32, PX_MAX_F32,
              PX_MAX_F32, PX_MAX_F32,
              PX_MAX_F32, PX_MAX_F32
            };

            PxVehicleDriveNWRawInputData carRawInput;
            carRawInput.setAnalogAccel(car->analogAcceleration);
            carRawInput.setAnalogBrake(car->analogBrake);
            carRawInput.setAnalogSteer(car->analogSteer);

            physxCar->mDriveDynData.forceGearChange(static_cast<uint32>(car->gear));

            const bool isInAir = physx::PxVehicleIsInAir(vehicleQueryResults[i]);

            PxFixedSizeLookupTable<8> steerVsForwardSpeedTable(steerVsForwardSpeedData, 4);
            PxVehicleDriveNWSmoothAnalogRawInputsAndSetAnalogInputs(smoothingData, steerVsForwardSpeedTable, carRawInput, timeElapsed, isInAir, *physxCar);

            car->ResetInputData();
        }
        else if (vehicleComponent->GetType()->Is<VehicleTankComponent>())
        {
            VehicleTankComponent* tank = static_cast<VehicleTankComponent*>(vehicleComponent);
            PxVehicleDriveTank* physxTank = static_cast<physx::PxVehicleDriveTank*>(vehicleComponent->vehicle);

            PxVehicleDriveTankRawInputData inputData(PxVehicleDriveTankControlModel::eSPECIAL);
            inputData.setAnalogAccel(tank->analogAcceleration);
            inputData.setAnalogLeftBrake(tank->analogLeftBrake);
            inputData.setAnalogRightBrake(tank->analogRightBrake);
            inputData.setAnalogLeftThrust(tank->analogLeftThrust);
            inputData.setAnalogRightThrust(tank->analogRightThrust);
            physxTank->mDriveDynData.forceGearChange(static_cast<uint32>(tank->gear));

            PxVehiclePadSmoothingData smoothingData =
            {
              {
              6.0f, //rise rate eANALOG_INPUT_ACCEL=0,
              6.0f, //rise rate eANALOG_INPUT_BRAKE,
              6.0f, //rise rate eANALOG_INPUT_HANDBRAKE,
              2.5f, //rise rate eANALOG_INPUT_STEER_LEFT,
              2.5f, //rise rate eANALOG_INPUT_STEER_RIGHT,
              },
              {
              10.0f, //fall rate eANALOG_INPUT_ACCEL=0
              10.0f, //fall rate eANALOG_INPUT_BRAKE_LEFT
              10.0f, //fall rate eANALOG_INPUT_BRAKE_RIGHT
              5.0f, //fall rate eANALOG_INPUT_THRUST_LEFT
              5.0f //fall rate eANALOG_INPUT_THRUST_RIGHT
              }
            };

            PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs(smoothingData, inputData, timeElapsed, *physxTank);

            tank->ResetInputData();
        }
    }
}

void PhysicsVehiclesSubsystem::OnSimulationEnabled(bool enabled)
{
    simulationEnabled = enabled;

    if (simulationEnabled)
    {
        for (VehicleCarComponent* carComponent : cars->components)
        {
            if (carComponent->vehicle == nullptr)
            {
                TryRecreateCarVehicle(carComponent);
            }
        }

        for (VehicleTankComponent* tankComponent : tanks->components)
        {
            if (tankComponent->vehicle == nullptr)
            {
                TryRecreateTankVehicle(static_cast<VehicleTankComponent*>(tankComponent));
            }
        }
    }
}

void PhysicsVehiclesSubsystem::SaveSimulationParams(VehicleCarComponent* car)
{
    physx::PxVehicleDrive* pxVehicleDrive = car->GetPxVehicle();
    physx::PxVehicleDriveNW* pxVehicleDriveNW = static_cast<physx::PxVehicleDriveNW*>(pxVehicleDrive);
    if (pxVehicleDriveNW != nullptr)
    {
        car->engineSpeed = pxVehicleDriveNW->mDriveDynData.getEngineRotationSpeed();

        for (uint32 i = 0; i < physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS; ++i)
        {
            car->analogInputStates[i] = pxVehicleDriveNW->mDriveDynData.getAnalogInput(i);
        }

        float32 jounces[4];
        pxVehicleDriveNW->mWheelsDynData.getWheels4InternalJounces(&jounces[0]);

        for (uint32 i = 0; i < 4; ++i)
        {
            VehicleWheelComponent* w = car->GetEntity()->GetComponent<VehicleWheelComponent>(i);

            w->jounce = FLOAT_EQUAL_EPS(jounces[i], FLOAT_MAX, 100.0f) ? 0.0f : jounces[i];
            pxVehicleDriveNW->mWheelsDynData.getWheelRotationSpeed(i, w->rotationSpeed, w->correctedRotationSpeed);
            w->rotationAngle = pxVehicleDriveNW->mWheelsDynData.getWheelRotationAngle(i);
        }
    }
}

void PhysicsVehiclesSubsystem::RestoreSimulationParams(VehicleCarComponent* car)
{
    physx::PxVehicleDrive* pxVehicleDrive = car->GetPxVehicle();
    physx::PxVehicleDriveNW* pxVehicleDriveNW = static_cast<physx::PxVehicleDriveNW*>(pxVehicleDrive);
    if (pxVehicleDriveNW != nullptr)
    {
        pxVehicleDriveNW->mDriveDynData.setEngineRotationSpeed(car->engineSpeed);

        for (uint32 i = 0; i < physx::PxVehicleDriveNWControl::eMAX_NB_DRIVENW_ANALOG_INPUTS; ++i)
        {
            pxVehicleDriveNW->mDriveDynData.setAnalogInput(i, car->analogInputStates[i]);
        }

        pxVehicleDriveNW->mDriveDynData.setCurrentGear(static_cast<physx::PxU32>(car->GetGear()));

        float32 jounces[4];
        for (uint32 i = 0; i < 4; ++i)
        {
            VehicleWheelComponent* w = car->GetEntity()->GetComponent<VehicleWheelComponent>(i);

            jounces[i] = w->jounce;

            pxVehicleDriveNW->mWheelsDynData.setWheelRotationSpeed(i, w->rotationSpeed, w->correctedRotationSpeed);
            pxVehicleDriveNW->mWheelsDynData.setWheelRotationAngle(i, w->rotationAngle);
        }

        pxVehicleDriveNW->mWheelsDynData.setWheels4InternalJounces(&jounces[0]);
    }
}

void PhysicsVehiclesSubsystem::SetupNonDrivableSurface(CollisionShapeComponent* surfaceShape) const
{
    DVASSERT(surfaceShape != nullptr);

    physx::PxShape* pxShape = surfaceShape->GetPxShape();
    DVASSERT(pxShape != nullptr);
    physx::PxFilterData filterData = pxShape->getQueryFilterData();
    filterData.word3 = UNDRIVABLE_SURFACE_FILTER;
    pxShape->setQueryFilterData(filterData);

    surfaceShape->SetTypeMask(GROUND_TYPE);
    surfaceShape->SetTypeMaskToCollideWith(GROUND_TYPES_TO_COLLIDE_WITH);
}

void PhysicsVehiclesSubsystem::SetupDrivableSurface(CollisionShapeComponent* surfaceShape) const
{
    DVASSERT(surfaceShape != nullptr);

    physx::PxShape* pxShape = surfaceShape->GetPxShape();
    DVASSERT(pxShape != nullptr);
    physx::PxFilterData filterData = pxShape->getQueryFilterData();
    filterData.word3 = DRIVABLE_SURFACE_FILTER;
    pxShape->setQueryFilterData(filterData);

    surfaceShape->SetTypeMask(GROUND_TYPE);
    surfaceShape->SetTypeMaskToCollideWith(GROUND_TYPES_TO_COLLIDE_WITH);
}

VehicleChassisComponent* PhysicsVehiclesSubsystem::GetChassis(VehicleComponent* vehicle) const
{
    DVASSERT(vehicle != nullptr);

    Entity* entity = vehicle->GetEntity();
    DVASSERT(entity != nullptr);

    return entity->GetComponent<VehicleChassisComponent>();
}

Vector<VehicleWheelComponent*> PhysicsVehiclesSubsystem::GetWheels(VehicleComponent* vehicle) const
{
    DVASSERT(vehicle != nullptr);

    Entity* entity = vehicle->GetEntity();
    DVASSERT(entity != nullptr);

    const int32 wheelsCount = entity->GetComponentCount<VehicleWheelComponent>();

    Vector<VehicleWheelComponent*> wheels;
    wheels.reserve(wheelsCount);

    for (int32 i = 0; i < wheelsCount; ++i)
    {
        VehicleWheelComponent* wheel = entity->GetComponent<VehicleWheelComponent>(i);
        DVASSERT(wheel != nullptr);

        wheels.push_back(wheel);
    }

    return wheels;
}

Vector3 PhysicsVehiclesSubsystem::CalculateMomentOfInertiaForShape(CollisionShapeComponent* shape)
{
    Vector3 momentOfInertia;

    if (shape->GetType()->Is<BoxShapeComponent>())
    {
        BoxShapeComponent* box = static_cast<BoxShapeComponent*>(shape);
        Vector3 boxFullSize(box->GetHalfSize() * 2.0f);
        momentOfInertia.x = (boxFullSize.z * boxFullSize.z + boxFullSize.x * boxFullSize.x) * box->GetMass() / 12.0f;
        momentOfInertia.y = (boxFullSize.y * boxFullSize.y + boxFullSize.x * boxFullSize.x) * box->GetMass() / 12.0f;
        momentOfInertia.z = (boxFullSize.y * boxFullSize.y + boxFullSize.z * boxFullSize.z) * box->GetMass() / 12.0f;
    }
    else
    {
        DVASSERT(false);
    }

    return momentOfInertia;
}

bool PhysicsVehiclesSubsystem::TryCreateVehicleCommonParts(
VehicleComponent* vehicleComponent,
float32 wheelMaxCompression,
float32 wheelMaxDroop,
float32 wheelSpringStrength,
float32 wheelSpringDamperRate,
uint32* outWheelsCount,
physx::PxVehicleWheelsSimData** outWheelsSimulationData)
{
    using namespace physx;

    DVASSERT(vehicleComponent != nullptr);

    Entity* vehicleEntity = vehicleComponent->GetEntity();
    DVASSERT(vehicleEntity != nullptr);

    // Check if entity has all components we need

    DynamicBodyComponent* vehicleBodyComponent = vehicleEntity->GetComponent<DynamicBodyComponent>();
    if (vehicleBodyComponent == nullptr)
    {
        return false;
    }

    VehicleChassisComponent* chassis = GetChassis(vehicleComponent);
    if (chassis == nullptr)
    {
        return false;
    }

    Vector<VehicleWheelComponent*> wheels = GetWheels(vehicleComponent);
    const uint32 wheelsCount = static_cast<uint32>(wheels.size());
    if (wheelsCount == 0)
    {
        return false;
    }

    Vector<CollisionShapeComponent*> vehicleShapes = PhysicsUtils::GetShapeComponents(vehicleEntity);
    if (vehicleShapes.size() != wheelsCount + 1) // One shape for chassis and one for each wheel
    {
        return false;
    }

    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        VehicleWheelComponent* wheel = wheels[i];
        ConvexHullShapeComponent* wheelShape = vehicleEntity->GetComponent<ConvexHullShapeComponent>(i);

        if (wheelShape->GetPxShape() == nullptr)
        {
            return false;
        }
    }

    PxRigidActor* vehicleActor = vehicleBodyComponent->GetPxActor();
    DVASSERT(vehicleActor != nullptr);

    PxRigidDynamic* vehicleRigidActor = vehicleActor->is<PxRigidDynamic>();
    DVASSERT(vehicleRigidActor != nullptr);

    // Chassis setup

    CollisionShapeComponent* chassisShape = vehicleEntity->GetComponent<BoxShapeComponent>();
    DVASSERT(chassisShape != nullptr);

    PxVehicleChassisData chassisData;
    chassisData.mCMOffset = PhysicsMath::Vector3ToPxVec3(chassis->GetCenterOfMassOffset());
    chassisData.mMass = chassisShape->GetMass();
    chassisData.mMOI = PhysicsMath::Vector3ToPxVec3(CalculateMomentOfInertiaForShape(chassisShape));

    PxShape* chassisShapePhysx = chassisShape->GetPxShape();
    DVASSERT(chassisShapePhysx != nullptr);

    PxFilterData filterData = chassisShapePhysx->getQueryFilterData();
    filterData.word3 = UNDRIVABLE_SURFACE_FILTER;
    chassisShapePhysx->setQueryFilterData(filterData);
    chassisShape->SetTypeMask(CHASSIS_TYPE);
    chassisShape->SetTypeMaskToCollideWith(CHASSIS_TYPES_TO_COLLIDE_WITH);

    vehicleRigidActor->setMass(chassisData.mMass);
    vehicleRigidActor->setMassSpaceInertiaTensor(chassisData.mMOI);
    vehicleRigidActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat(PxIdentity)));

    // Wheels setup

    SkeletonComponent* skeletonComponent = vehicleEntity->GetComponent<SkeletonComponent>();

    PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(wheelsCount);
    PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
    PxVehicleWheelData wheelsData[PX_MAX_NB_WHEELS];
    PxVehicleTireData tiresData[PX_MAX_NB_WHEELS];
    PxVec3 suspensionTravelDirections[PX_MAX_NB_WHEELS];
    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        VehicleWheelComponent* wheel = wheels[i];
        ConvexHullShapeComponent* wheelShape = vehicleEntity->GetComponent<ConvexHullShapeComponent>(i);

        PxShape* wheelShapePhysx = wheelShape->GetPxShape();
        DVASSERT(wheelShapePhysx != nullptr);

        PxFilterData filterData = wheelShapePhysx->getQueryFilterData();
        filterData.word3 = UNDRIVABLE_SURFACE_FILTER;
        wheelShapePhysx->setQueryFilterData(filterData);
        wheelShape->SetTypeMask(WHEEL_TYPE);
        wheelShape->SetTypeMaskToCollideWith(WHEEL_TYPES_TO_COLLIDE_WITH);

        const FastName& jointName = wheelShape->GetJointName();
        DVASSERT(jointName.IsValid() && jointName.size() > 0);

        uint32 jointIndex = skeletonComponent->GetJointIndex(jointName);

        Vector3 jointPosition = skeletonComponent->GetDefaultPose().GetJointTransform(jointIndex).GetPosition();
        wheelCenterActorOffsets[i] = PhysicsMath::Vector3ToPxVec3(jointPosition);

        wheelsData[i].mMass = wheelShape->GetMass();
        wheelsData[i].mMOI = 0.5f * (wheel->GetRadius() * wheel->GetRadius()) * wheelsData[i].mMass; // Moment of inertia for a cylinder
        wheelsData[i].mRadius = wheel->GetRadius();
        wheelsData[i].mWidth = wheel->GetWidth();
        wheelsData[i].mMaxHandBrakeTorque = wheel->GetMaxHandbrakeTorque();
        wheelsData[i].mMaxSteer = wheel->GetMaxSteerAngle();
        wheelsData[i].mDampingRate = 2.0f;

        tiresData[i].mType = TIRE_TYPE_NORMAL;
    }

    PxF32 suspensionSprungMasses[PX_MAX_NB_WHEELS];
    PxVehicleComputeSprungMasses(wheelsCount, wheelCenterActorOffsets, chassisData.mCMOffset, chassisData.mMass, 2 /* = (0, 0, -1) gravity */, suspensionSprungMasses);

    PxVehicleSuspensionData suspensionsData[PX_MAX_NB_WHEELS];
    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        suspensionsData[i].mMaxCompression = wheelMaxCompression;
        suspensionsData[i].mMaxDroop = wheelMaxDroop;
        suspensionsData[i].mSpringStrength = wheelSpringStrength;
        suspensionsData[i].mSpringDamperRate = wheelSpringDamperRate;
        suspensionsData[i].mSprungMass = suspensionSprungMasses[i];
    }

    PxVec3 suspensionDirection = pxScene->getGravity();
    suspensionDirection.normalize();

    PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 suspensionForceAppCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];

    PxShape* physxShapes[PX_MAX_NB_WHEELS + 1];
    vehicleActor->getShapes(&physxShapes[0], PX_MAX_NB_WHEELS + 1, 0);

    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        ConvexHullShapeComponent* wheelShape = vehicleEntity->GetComponent<ConvexHullShapeComponent>(i);

        for (uint32 j = 0; j < vehicleShapes.size(); ++j)
        {
            if (physxShapes[j] == wheelShape->GetPxShape())
            {
                wheelsSimData->setWheelShapeMapping(i, j);
            }
        }

        suspensionTravelDirections[i] = suspensionDirection;
        wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - chassisData.mCMOffset;
        suspensionForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, wheelCentreCMOffsets[i].y, -0.3f);
        tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, wheelCentreCMOffsets[i].y, -0.3f);

        wheelsSimData->setWheelData(i, wheelsData[i]);
        wheelsSimData->setTireData(i, tiresData[i]);
        wheelsSimData->setSuspensionData(i, suspensionsData[i]);
        wheelsSimData->setSuspTravelDirection(i, suspensionTravelDirections[i]);
        wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsSimData->setSuspForceAppPointOffset(i, suspensionForceAppCMOffsets[i]);
        wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
        wheelsSimData->setSceneQueryFilterData(i, PxFilterData(0, WHEEL_TYPE, WHEEL_TYPES_TO_COLLIDE_WITH, UNDRIVABLE_SURFACE_FILTER));
        wheelsSimData->setSubStepCount(1, 6, 2);
    }

    *outWheelsCount = wheelsCount;
    *outWheelsSimulationData = wheelsSimData;

    return true;
}

void PhysicsVehiclesSubsystem::TryRecreateCarVehicle(VehicleCarComponent* vehicleComponent)
{
    using namespace physx;

    DVASSERT(vehicleComponent != nullptr);

    Entity* vehicleEntity = vehicleComponent->GetEntity();
    DVASSERT(vehicleEntity != nullptr);

    if (vehicleComponent->vehicle != nullptr)
    {
        static_cast<physx::PxVehicleDriveNW*>(vehicleComponent->vehicle)->free();
        vehicleComponent->vehicle = nullptr;
    }

    const float32 wheelMaxCompression = 0.3f;
    const float32 wheelMaxDroop = 0.1f;
    const float32 wheelSpringStrength = 35000.0f;
    const float32 wheelSpringDamperRate = 4500.0f;

    uint32 wheelsCount;
    PxVehicleWheelsSimData* wheelsSimData;
    if (!TryCreateVehicleCommonParts(vehicleComponent, wheelMaxCompression, wheelMaxDroop, wheelSpringStrength, wheelSpringDamperRate, &wheelsCount, &wheelsSimData))
    {
        return;
    }

    PxVehicleDriveSimDataNW driveSimData;

    PxVehicleDifferentialNWData diff;
    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        diff.setDrivenWheel(i, true);
    }
    driveSimData.setDiffData(diff);

    PxVehicleEngineData engine;
    engine.mPeakTorque = 500.0f;
    engine.mMaxOmega = 600.0f;
    driveSimData.setEngineData(engine);

    PxVehicleGearsData gears;
    gears.mSwitchTime = 0.5f;
    driveSimData.setGearsData(gears);

    PxVehicleClutchData clutch;
    clutch.mStrength = 10.0f;
    driveSimData.setClutchData(clutch);

    PxVehicleDriveNW* vehDrive4W = PxVehicleDriveNW::allocate(wheelsCount);

    PhysicsComponent* vehiclePhysicsComponent = static_cast<PhysicsComponent*>(vehicleEntity->GetComponent<DynamicBodyComponent>());
    DVASSERT(vehiclePhysicsComponent != nullptr);
    PxActor* vehicleActor = vehiclePhysicsComponent->GetPxActor();
    DVASSERT(vehicleActor != nullptr);
    PxRigidDynamic* vehicleRigidActor = vehicleActor->is<PxRigidDynamic>();

    // Debug log

    VehicleChassisComponent* chassis = GetChassis(vehicleComponent);
    DVASSERT(chassis != nullptr);

    DynamicBodyComponent* vehicleBodyComponent = vehicleEntity->GetComponent<DynamicBodyComponent>();
    DVASSERT(vehicleBodyComponent != nullptr);

    PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("Creating car vehicle with params:");

    PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("vehicleRigidActor->getMass() = %f", vehicleRigidActor->getMass());
    PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("vehicleRigidActor->getMassSpaceInertiaTensor() = %f, %f, %f", vehicleRigidActor->getMassSpaceInertiaTensor().x, vehicleRigidActor->getMassSpaceInertiaTensor().y, vehicleRigidActor->getMassSpaceInertiaTensor().z);
    PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("vehicleRigidActor->getCMassLocalPose() = %f, %f, %f; %f, %f, %f, %f", vehicleRigidActor->getCMassLocalPose().p.x, vehicleRigidActor->getCMassLocalPose().p.y, vehicleRigidActor->getCMassLocalPose().p.z, vehicleRigidActor->getCMassLocalPose().q.x, vehicleRigidActor->getCMassLocalPose().q.y, vehicleRigidActor->getCMassLocalPose().q.z, vehicleRigidActor->getCMassLocalPose().q.w);

    for (uint32 i = 0; i < wheelsCount; ++i)
    {
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mDampingRate = %f", i, wheelsSimData->getWheelData(i).mDampingRate);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mMass = %f", i, wheelsSimData->getWheelData(i).mMass);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mMaxBrakeTorque = %f", i, wheelsSimData->getWheelData(i).mMaxBrakeTorque);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mMaxHandBrakeTorque = %f", i, wheelsSimData->getWheelData(i).mMaxHandBrakeTorque);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mMaxSteer = %f", i, wheelsSimData->getWheelData(i).mMaxSteer);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mMOI = %f", i, wheelsSimData->getWheelData(i).mMOI);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mRadius = %f", i, wheelsSimData->getWheelData(i).mRadius);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mToeAngle = %f", i, wheelsSimData->getWheelData(i).mToeAngle);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelData(%u).mWidth = %f", i, wheelsSimData->getWheelData(i).mWidth);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mCamberStiffnessPerUnitGravity = %f", i, wheelsSimData->getTireData(i).mCamberStiffnessPerUnitGravity);
        
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mFrictionVsSlipGraph = [%f, %f, %f, %f, %f, %f]", i, wheelsSimData->getTireData(i).mFrictionVsSlipGraph[0][0], wheelsSimData->getTireData(i).mFrictionVsSlipGraph[0][1], wheelsSimData->getTireData(i).mFrictionVsSlipGraph[1][0], wheelsSimData->getTireData(i).mFrictionVsSlipGraph[1][1], wheelsSimData->getTireData(i).mFrictionVsSlipGraph[2][0], wheelsSimData->getTireData(i).mFrictionVsSlipGraph[2][1]);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mLatStiffX = %f", i, wheelsSimData->getTireData(i).mLatStiffX);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mLatStiffY = %f", i, wheelsSimData->getTireData(i).mLatStiffY);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mLongitudinalStiffnessPerUnitGravity = %f", i, wheelsSimData->getTireData(i).mLongitudinalStiffnessPerUnitGravity);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireData(%u).mCamberStiffnessPerUnitGravity = %u", i, wheelsSimData->getTireData(i).mType);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mCamberAtMaxCompression = %f", i, wheelsSimData->getSuspensionData(i).mCamberAtMaxCompression);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mCamberAtMaxDroop = %f", i, wheelsSimData->getSuspensionData(i).mCamberAtMaxDroop);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mCamberAtRest = %f", i, wheelsSimData->getSuspensionData(i).mCamberAtRest);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mMaxCompression = %f", i, wheelsSimData->getSuspensionData(i).mMaxCompression);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mMaxDroop = %f", i, wheelsSimData->getSuspensionData(i).mMaxDroop);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mSpringDamperRate = %f", i, wheelsSimData->getSuspensionData(i).mSpringDamperRate);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mSpringStrength = %f", i, wheelsSimData->getSuspensionData(i).mSpringStrength);
        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspensionData(%u).mSprungMass = %f", i, wheelsSimData->getSuspensionData(i).mSprungMass);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspTravelDirection(%u) = %f, %f, %f", i, wheelsSimData->getSuspTravelDirection(i).x, wheelsSimData->getSuspTravelDirection(i).y, wheelsSimData->getSuspTravelDirection(i).z);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getWheelCentreOffset(%u) = %f, %f, %f", i, wheelsSimData->getWheelCentreOffset(i).x, wheelsSimData->getWheelCentreOffset(i).y, wheelsSimData->getWheelCentreOffset(i).z);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getSuspForceAppPointOffset(%u) = %f, %f, %f", i, wheelsSimData->getSuspForceAppPointOffset(i).x, wheelsSimData->getSuspForceAppPointOffset(i).y, wheelsSimData->getSuspForceAppPointOffset(i).z);

        PHYSICS_VEHICLES_SYSTEM_DEBUG_LOG("\twheelsSimData->.getTireForceAppPointOffset(%u) = %f, %f, %f", i, wheelsSimData->getTireForceAppPointOffset(i).x, wheelsSimData->getTireForceAppPointOffset(i).y, wheelsSimData->getTireForceAppPointOffset(i).z);
    }

    //

    vehDrive4W->setup(&PxGetPhysics(), vehicleRigidActor, *wheelsSimData, driveSimData, wheelsCount);
    vehicleComponent->vehicle = vehDrive4W;

    vehDrive4W->setToRestState();
    vehDrive4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
    wheelsSimData->free();
}

void PhysicsVehiclesSubsystem::TryRecreateTankVehicle(VehicleTankComponent* vehicleComponent)
{
    using namespace physx;

    DVASSERT(vehicleComponent != nullptr);

    Entity* vehicleEntity = vehicleComponent->GetEntity();
    DVASSERT(vehicleEntity != nullptr);

    if (vehicleComponent->vehicle != nullptr)
    {
        static_cast<physx::PxVehicleDriveNW*>(vehicleComponent->vehicle)->free();
        vehicleComponent->vehicle = nullptr;
    }

    const float32 wheelMaxCompression = 0.3f;
    const float32 wheelMaxDroop = 0.1f;
    const float32 wheelSpringStrength = 10000.0f;
    const float32 wheelSpringDamperRate = 5000.0f;

    uint32 wheelsCount;
    PxVehicleWheelsSimData* wheelsSimData;
    if (!TryCreateVehicleCommonParts(vehicleComponent, wheelMaxCompression, wheelMaxDroop, wheelSpringStrength, wheelSpringDamperRate, &wheelsCount, &wheelsSimData))
    {
        return;
    }

    PxVehicleDriveSimData driveSimData;

    PxVehicleEngineData engineData = driveSimData.getEngineData();
    engineData.mDampingRateZeroThrottleClutchEngaged = 3.0f;
    engineData.mDampingRateZeroThrottleClutchDisengaged = 2.0f;
    engineData.mDampingRateFullThrottle = 1.0f;
    driveSimData.setEngineData(engineData);

    PhysicsComponent* vehiclePhysicsComponent = static_cast<PhysicsComponent*>(vehicleEntity->GetComponent<DynamicBodyComponent>());
    DVASSERT(vehiclePhysicsComponent != nullptr);
    PxActor* vehicleActor = vehiclePhysicsComponent->GetPxActor();
    DVASSERT(vehicleActor != nullptr);
    PxRigidDynamic* vehicleRigidActor = vehicleActor->is<PxRigidDynamic>();

    PxVehicleDriveTank* vehicleTank = PxVehicleDriveTank::allocate(wheelsCount);
    vehicleTank->setup(&PxGetPhysics(), vehicleRigidActor, *wheelsSimData, driveSimData, wheelsCount);
    vehicleComponent->vehicle = vehicleTank;

    vehicleTank->setToRestState();
    vehicleTank->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
    vehicleTank->mDriveDynData.setUseAutoGears(true);
    vehicleTank->setDriveModel(PxVehicleDriveTankControlModel::eSPECIAL);

    wheelsSimData->free();
}
}
