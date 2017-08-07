#include "Physics/Private/PhysicsVehiclesSubsystem.h"

#include "Physics/VehicleComponent.h"
#include "Physics/VehicleChassisComponent.h"
#include "Physics/VehicleWheelComponent.h"

#include <Input/InputSystem.h>
#include <Time/SystemTimer.h>

#include <physx/PxScene.h>
#include <physx/vehicle/PxVehicleUtil.h>

#include <algorithm>

namespace DAVA
{
const uint32 MAX_VEHICLES_COUNT = 100;

const uint32 COLLISION_FLAG_GROUND = 1 << 0;
const uint32 COLLISION_FLAG_WHEEL = 1 << 1;
const uint32 COLLISION_FLAG_CHASSIS = 1 << 2;
const uint32 COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS;
const uint32 COLLISION_FLAG_WHEEL_AGAINST = COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS;
const uint32 COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS;

const uint32 DRIVABLE_SURFACE_FILTER = 0xffff0000;
const uint32 UNDRIVABLE_SURFACE_FILTER = 0x0000ffff;

const uint32 SURFACE_TYPE_NORMAL = 0;

const uint32 TIRE_TYPE_NORMAL = 0;

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
    static physx::PxBatchQuery* SsetUpBatchedSceneQuery(const physx::PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene);

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
    ,
    numHitResultsPerQuery(0)
    ,
    raycastResults(NULL)
    ,
    raycastHitBuffer(NULL)
    ,
    preFilterShader(NULL)
    ,
    postFilterShader(NULL)
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
    PxU8* buffer = static_cast<PxU8*>(allocator.allocate(size, NULL, NULL, 0));

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

physx::PxBatchQuery* VehicleSceneQueryData::SsetUpBatchedSceneQuery(const physx::PxU32 batchId, const VehicleSceneQueryData& vehicleSceneQueryData, physx::PxScene* scene)
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

physx::PxVehicleDrivableSurfaceToTireFrictionPairs* createFrictionPairs(const physx::PxMaterial* defaultMaterial)
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

physx::PxQueryHitType::Enum WheelSceneQueryPreFilterBlocking
(physx::PxFilterData filterData0, physx::PxFilterData filterData1,
 const void* constantBlock, physx::PxU32 constantBlockSize,
 physx::PxHitFlags& queryFlags)
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

PhysicsVehiclesSubsystem::PhysicsVehiclesSubsystem(Scene* scene, physx::PxScene* pxScene)
    : pxScene(pxScene)
{
}
PhysicsVehiclesSubsystem::~PhysicsVehiclesSubsystem()
{
}

void PhysicsVehiclesSubsystem::RegisterEntity(Entity* entity)
{
    DVASSERT(entity != nullptr);

    VehicleComponent* vehicleComponent = static_cast<VehicleComponent*>(entity->GetComponent(Component::VEHICLE_COMPONENT));
    if (vehicleComponent != nullptr)
    {
        RegisterComponent(entity, vehicleComponent);
    }
}

void PhysicsVehiclesSubsystem::UnregisterEntity(Entity* entity)
{
    DVASSERT(entity != nullptr);

    VehicleComponent* vehicleComponent = static_cast<VehicleComponent*>(entity->GetComponent(Component::VEHICLE_COMPONENT));
    if (vehicleComponent != nullptr)
    {
        UnregisterComponent(entity, vehicleComponent);
    }
}

void PhysicsVehiclesSubsystem::RegisterComponent(Entity* entity, Component* component)
{
    DVASSERT(entity != nullptr);
    DVASSERT(component != nullptr);

    if (component->GetType() == Component::VEHICLE_COMPONENT)
    {
        DVASSERT(std::find(vehicleComponents.begin(), vehicleComponents.end(), component) == vehicleComponents.end());

        vehicleComponents.push_back(static_cast<VehicleComponent*>(component));
    }
}

void PhysicsVehiclesSubsystem::UnregisterComponent(Entity* entity, Component* component)
{
    DVASSERT(entity != nullptr);
    DVASSERT(component != nullptr);

    if (component->GetType() == Component::VEHICLE_COMPONENT)
    {
        DVASSERT(std::find(vehicleComponents.begin(), vehicleComponents.end(), component) != vehicleComponents.end());

        vehicleComponents.erase(std::remove(vehicleComponents.begin(), vehicleComponents.end(), component), vehicleComponents.end());
    }
}

void PhysicsVehiclesSubsystem::Simulate(float32 timeElapsed)
{
    using namespace physx;

    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();

    static VehicleSceneQueryData* vehicleSceneQueryData = VehicleSceneQueryData::Allocate(1, PX_MAX_NB_WHEELS, 1, 1, WheelSceneQueryPreFilterBlocking, NULL, PxDefaultAllocator());
    ;
    static physx::PxBatchQuery* batchQuery = VehicleSceneQueryData::SsetUpBatchedSceneQuery(0, *vehicleSceneQueryData, pxScene);
    static PxVehicleDrivableSurfaceToTireFrictionPairs* frictionPairs = createFrictionPairs(physics->GetDefaultMaterial());

    static PxVehicleWheels* physxVehicles[MAX_VEHICLES_COUNT];
    static PxVehicleWheelQueryResult vehicleQueryResults[MAX_VEHICLES_COUNT];
    static PxWheelQueryResult wheelQueryResults[MAX_VEHICLES_COUNT][PX_MAX_NB_WHEELS];
    for (int i = 0; i < vehicleComponents.size(); ++i)
    {
        physxVehicles[i] = vehicleComponents[i]->vehicle;

        vehicleQueryResults[i].nbWheelQueryResults = physxVehicles[i]->mWheelsSimData.getNbWheels();
        vehicleQueryResults[i].wheelQueryResults = wheelQueryResults[i];
    }

    // Raycasts
    PxRaycastQueryResult* raycastResults = vehicleSceneQueryData->GetRaycastQueryResultBuffer(0);
    const PxU32 raycastResultsSize = vehicleSceneQueryData->GetQueryResultBufferSize();
    PxVehicleSuspensionRaycasts(batchQuery, static_cast<physx::PxU32>(vehicleComponents.size()), physxVehicles, raycastResultsSize, raycastResults);

    // Vehicle update
    const PxVec3 grav = pxScene->getGravity();
    PxVehicleUpdates(SystemTimer::GetRealFrameDelta(), grav, *frictionPairs, static_cast<physx::PxU32>(vehicleComponents.size()), physxVehicles, vehicleQueryResults);

    // Process input
    // TODO: move it out of here

    static PxVehicleKeySmoothingData keySmoothingData =
    {
      {
      3.0f, // rise rate eANALOG_INPUT_ACCEL
      3.0f, // rise rate eANALOG_INPUT_BRAKE
      10.0f, // rise rate eANALOG_INPUT_HANDBRAKE
      2.5f, // rise rate eANALOG_INPUT_STEER_LEFT
      2.5f, // rise rate eANALOG_INPUT_STEER_RIGHT
      },
      {
      5.0f, // fall rate eANALOG_INPUT__ACCEL
      5.0f, // fall rate eANALOG_INPUT__BRAKE
      10.0f, // fall rate eANALOG_INPUT__HANDBRAKE
      5.0f, // fall rate eANALOG_INPUT_STEER_LEFT
      5.0f // fall rate eANALOG_INPUT_STEER_RIGHT
      }
    };

    static PxF32 gSteerVsForwardSpeedData[2 * 8] =
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

    for (VehicleComponent* vehicleComponent : vehicleComponents)
    {
        vehicleComponent->ResetInputData();

        KeyboardDevice& kb = InputSystem::Instance()->GetKeyboard();
        if (kb.IsKeyPressed(Key::KEY_Y))
        {
            vehicleComponent->SetDigitalAcceleration(true);
        }
        if (kb.IsKeyPressed(Key::KEY_H))
        {
            vehicleComponent->SetDigitalBrake(true);
        }
        if (kb.IsKeyPressed(Key::KEY_G))
        {
            vehicleComponent->SetDigitalSteerRight(true);
        }
        if (kb.IsKeyPressed(Key::KEY_J))
        {
            vehicleComponent->SetDigitalSteerLeft(true);
        }

        const bool isInAir = physx::PxVehicleIsInAir(vehicleQueryResults[0]);

        PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData, 4);
        PxVehicleDriveNWRawInputData carRawInputs = vehicleComponent->GetRawInputData();
        PxVehicleDriveNWSmoothDigitalRawInputsAndSetAnalogInputs(keySmoothingData, gSteerVsForwardSpeedTable, carRawInputs, SystemTimer::GetRealFrameDelta(), isInAir, *vehicleComponent->vehicle);
    }
}

void PhysicsVehiclesSubsystem::OnSimulationEnabled(bool enabled)
{
    simulationEnabled = enabled;

    if (simulationEnabled)
    {
        for (VehicleComponent* vehicleComponent : vehicleComponents)
        {
            CreatePhysxVehicle(vehicleComponent);
        }
    }
}

void PhysicsVehiclesSubsystem::SetupDrivableSurface(physx::PxShape* surfaceShape) const
{
    surfaceShape->setQueryFilterData(physx::PxFilterData(0, 0, 0, DRIVABLE_SURFACE_FILTER));
    surfaceShape->setSimulationFilterData(physx::PxFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0));
}

VehicleChassisComponent* PhysicsVehiclesSubsystem::GetChassis(VehicleComponent* vehicle) const
{
    Entity* entity = vehicle->GetEntity();
    DVASSERT(entity != nullptr);

    const size_t childrenCount = entity->GetChildrenCount();
    for (int i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        DVASSERT(child != nullptr);

        VehicleChassisComponent* chassis = static_cast<VehicleChassisComponent*>(child->GetComponent(Component::VEHICLE_CHASSIS_COMPONENT));
        if (chassis != nullptr)
        {
            return chassis;
        }
    }

    return nullptr;
}

Vector<VehicleWheelComponent*> PhysicsVehiclesSubsystem::GetWheels(VehicleComponent* vehicle) const
{
    Vector<VehicleWheelComponent*> wheels;

    Entity* entity = vehicle->GetEntity();
    DVASSERT(entity != nullptr);

    const size_t childrenCount = entity->GetChildrenCount();
    for (int i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        DVASSERT(child != nullptr);

        VehicleWheelComponent* wheel = static_cast<VehicleWheelComponent*>(child->GetComponent(Component::VEHICLE_WHEEL_COMPONENT));
        if (wheel != nullptr)
        {
            wheels.push_back(wheel);
        }
    }

    return wheels;
}

Vector3 PhysicsVehiclesSubsystem::CalculateMomentOfInertiaForShape(CollisionShapeComponent* shape)
{
    Vector3 momentOfInertia;

    switch (shape->GetType())
    {
    case Component::BOX_SHAPE_COMPONENT:
    {
        BoxShapeComponent* box = static_cast<BoxShapeComponent*>(shape);
        Vector3 boxFullSize(box->GetHalfSize() * 2.0f);
        momentOfInertia.x = (boxFullSize.z * boxFullSize.z + boxFullSize.x * boxFullSize.x) * box->GetMass() / 12.0f;
        momentOfInertia.y = (boxFullSize.y * boxFullSize.y + boxFullSize.x * boxFullSize.x) * box->GetMass() / 12.0f;
        momentOfInertia.z = (boxFullSize.y * boxFullSize.y + boxFullSize.z * boxFullSize.z) * box->GetMass() / 12.0f;
        break;
    }

    default:
    {
        DVASSERT(false);
        break;
    }
    }

    return momentOfInertia;
}

void PhysicsVehiclesSubsystem::CreatePhysxVehicle(VehicleComponent* vehicleComponent)
{
    using namespace physx;

    DVASSERT(vehicleComponent != nullptr);

    if (vehicleComponent->vehicle != nullptr)
    {
        vehicleComponent->vehicle->free();
        vehicleComponent->vehicle = nullptr;
    }

    // Chassis setup

    VehicleChassisComponent* chassis = GetChassis(vehicleComponent);
    DVASSERT(chassis != nullptr);

    Vector<CollisionShapeComponent*> chassisShapes = CollisionShapeComponent::GetFromEntity(chassis->GetEntity());
    DVASSERT(chassisShapes.size() == 1);
    CollisionShapeComponent* chassisShape = chassisShapes[0];

    PxVehicleChassisData chassisData;
    chassisData.mCMOffset = PhysicsMath::Vector3ToPxVec3(chassis->GetCenterOfMassOffset());
    chassisData.mMass = chassisShape->GetMass();
    chassisData.mMOI = PhysicsMath::Vector3ToPxVec3(CalculateMomentOfInertiaForShape(chassisShape));

    PxFilterData chassisQueryFilterData = PxFilterData(0, 0, 0, UNDRIVABLE_SURFACE_FILTER);
    PxFilterData chassisSimulationFilterData = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

    PxShape* chassisShapePhysx = chassisShape->GetPxShape();
    DVASSERT(chassisShapePhysx != nullptr);

    chassisShapePhysx->setQueryFilterData(chassisQueryFilterData);
    chassisShapePhysx->setSimulationFilterData(chassisSimulationFilterData);
    chassisShapePhysx->setLocalPose(PxTransform(PxIdentity));

    // Wheels setup

    Vector<VehicleWheelComponent*> wheels = GetWheels(vehicleComponent);
    DVASSERT(wheels.size() > 0);

    PxFilterData wheelQueryFilterData = PxFilterData(0, 0, 0, UNDRIVABLE_SURFACE_FILTER);
    PxFilterData wheelSimulationFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);
    PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(static_cast<PxU32>(wheels.size()));
    PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
    PxVehicleWheelData wheelsData[PX_MAX_NB_WHEELS];
    PxVehicleTireData tiresData[PX_MAX_NB_WHEELS];
    float32 wheelsTotalMass = 0.0f;
    for (int i = 0; i < wheels.size(); ++i)
    {
        VehicleWheelComponent* wheel = wheels[i];
        Vector<CollisionShapeComponent*> wheelShapes = CollisionShapeComponent::GetFromEntity(wheel->GetEntity());
        DVASSERT(wheelShapes.size() == 1);
        CollisionShapeComponent* wheelShape = wheelShapes[0];

        PxShape* wheelShapePhysx = wheelShape->GetPxShape();
        DVASSERT(wheelShapePhysx != nullptr);

        wheelShapePhysx->setQueryFilterData(wheelQueryFilterData);
        wheelShapePhysx->setSimulationFilterData(wheelSimulationFilterData);
        wheelShapePhysx->setLocalPose(PxTransform(PxIdentity));

        Matrix4 wheelLocalTransform = wheelShape->GetEntity()->GetLocalTransform();
        wheelCenterActorOffsets[i] = PhysicsMath::Vector3ToPxVec3(wheelLocalTransform.GetTranslationVector());

        wheelsData[i].mMass = wheelShape->GetMass();
        wheelsData[i].mMOI = 0.5f * (wheel->GetRadius() * wheel->GetRadius()) * wheelsData[i].mMass; // Moment of inertia for cylinder
        wheelsData[i].mRadius = wheel->GetRadius();
        wheelsData[i].mWidth = wheel->GetWidth();
        wheelsData[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
        wheelsData[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
        wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = PxPi * 0.3333f;
        wheelsData[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi * 0.3333f;

        tiresData[i].mType = TIRE_TYPE_NORMAL;

        wheelsTotalMass += wheelShape->GetMass();
    }

    PxF32 suspensionSprungMasses[PX_MAX_NB_WHEELS];
    PxVehicleComputeSprungMasses(static_cast<PxU32>(wheels.size()), wheelCenterActorOffsets, chassisData.mCMOffset, chassisData.mMass + wheelsTotalMass, 2 /* = 0, 0, -1 gravity */, suspensionSprungMasses);
    PxVehicleSuspensionData suspensionsData[PX_MAX_NB_WHEELS];
    for (int i = 0; i < wheels.size(); ++i)
    {
        suspensionsData[i].mMaxCompression = 0.3f;
        suspensionsData[i].mMaxDroop = 0.1f;
        suspensionsData[i].mSpringStrength = 35000.0f;
        suspensionsData[i].mSpringDamperRate = 4500.0f;
        suspensionsData[i].mSprungMass = suspensionSprungMasses[i];
    }

    const PxF32 camberAngleAtRest = 0.0;
    const PxF32 camberAngleAtMaxDroop = 0.01f;
    const PxF32 camberAngleAtMaxCompression = -0.01f;
    for (int i = 0; i < wheels.size(); i += 2)
    {
        suspensionsData[i + 0].mCamberAtRest = camberAngleAtRest;
        suspensionsData[i + 1].mCamberAtRest = -camberAngleAtRest;
        suspensionsData[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop;
        suspensionsData[i + 1].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
        suspensionsData[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression;
        suspensionsData[i + 1].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
    }

    PxVec3 suspensionDirection = pxScene->getGravity();
    suspensionDirection.normalize();

    PxVec3 suspensionTravelDirections[PX_MAX_NB_WHEELS];
    PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 suspensionForceAppCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
    for (int i = 0; i < wheels.size(); ++i)
    {
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
        wheelsSimData->setSceneQueryFilterData(i, wheelQueryFilterData);
        wheelsSimData->setWheelShapeMapping(i, PxI32(i));
    }

    PxVehicleAntiRollBarData barFront;
    barFront.mWheel0 = PxVehicleDrive4WWheelOrder::eFRONT_LEFT;
    barFront.mWheel1 = PxVehicleDrive4WWheelOrder::eFRONT_RIGHT;
    barFront.mStiffness = 0;
    wheelsSimData->addAntiRollBarData(barFront);

    PxVehicleAntiRollBarData barRear;
    barRear.mWheel0 = PxVehicleDrive4WWheelOrder::eREAR_LEFT;
    barRear.mWheel1 = PxVehicleDrive4WWheelOrder::eREAR_RIGHT;
    barRear.mStiffness = 0;
    wheelsSimData->addAntiRollBarData(barRear);

    // Actor setup

    PhysicsComponent* vehiclePhysicsComponent = static_cast<PhysicsComponent*>(vehicleComponent->GetEntity()->GetComponent(Component::DYNAMIC_BODY_COMPONENT));
    DVASSERT(vehiclePhysicsComponent != nullptr);

    PxActor* vehicleActor = vehiclePhysicsComponent->GetPxActor();
    DVASSERT(vehicleActor != nullptr);
    PxRigidDynamic* vehicleRigidActor = vehicleActor->is<PxRigidDynamic>();

    vehicleRigidActor->setMassSpaceInertiaTensor(chassisData.mMOI);
    vehicleRigidActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset, PxQuat(PxIdentity)));

    PxVehicleDriveSimDataNW driveSimData;

    PxVehicleDifferentialNWData diff;
    for (int i = 0; i < wheels.size(); ++i)
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

    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    PxVehicleDriveNW* vehDrive4W = PxVehicleDriveNW::allocate(static_cast<PxU32>(wheels.size()));
    vehDrive4W->setup(&PxGetPhysics(), vehicleRigidActor, *wheelsSimData, driveSimData, 0);
    vehicleComponent->vehicle = vehDrive4W;

    vehDrive4W->setToRestState();
    vehDrive4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
    vehDrive4W->mDriveDynData.setUseAutoGears(true);

    List<Entity*> meshNodes;
    vehicleComponent->GetEntity()->GetChildNodes(meshNodes);
    for (Entity* e : meshNodes)
    {
        CollisionShapeComponent* c = static_cast<CollisionShapeComponent*>(e->GetComponent(Component::CONVEX_HULL_SHAPE_COMPONENT));
        if (c == nullptr)
        {
            c = static_cast<CollisionShapeComponent*>(e->GetComponent(Component::BOX_SHAPE_COMPONENT));
        }
        physx::PxShape* shape = c->GetPxShape();

        Matrix4 pos = PhysicsMath::PxMat44ToMatrix4(shape->getLocalPose());
        e->SetLocalTransform(pos);
    }

    wheelsSimData->free();
}
}