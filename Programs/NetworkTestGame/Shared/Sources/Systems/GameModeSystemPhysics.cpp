#include "Systems/GameModeSystemPhysics.h"

#include <Engine/EngineContext.h>
#include <DeviceManager/DeviceManager.h>
#include <Logger/Logger.h>
#include <Entity/ComponentUtils.h>
#include <Scene3D/Entity.h>
#include <UI/UIControlSystem.h>
#include <Input/Keyboard.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/3D/PolygonGroup.h>
#include <Render/Material/NMaterial.h>
#include <Render/Highlevel/RenderObject.h>

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/UDPTransport/UDPServer.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/StaticBodyComponent.h>
#include <Physics/PlaneShapeComponent.h>
#include <Physics/BoxShapeComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameModeSystemPhysics)
{
    ReflectionRegistrator<GameModeSystemPhysics>::Begin()
    .ConstructorByPointer<Scene*>()
    //.Method("ProcessFixed", &GameModeSystemPhysics::ProcessFixed)
    .End();
}

namespace GameModeSystemPhysicsDetail
{
static const uint32 WALL_COLLISION_SHAPE_TYPE = 1 << 0;
static const uint32 CUBE_COLLISION_SHAPE_TYPE = 1 << 1;

static const FastName ACTION_FORWARD = FastName("FORWARD");
static const FastName ACTION_BACK = FastName("BACK");
static const FastName ACTION_LEFT = FastName("LEFT");
static const FastName ACTION_RIGHT = FastName("RIGHT");

static const float32 ROOM_SIZE = 30.0f;
static const float32 SMALL_CUBE_HALF_SIZE = 0.25f;
static const float32 BIG_CUBE_HALF_SIZE = 0.75f;

static const uint32 SMALL_CUBES_GRID_SIZE = 15;

static const float32 BIG_CUBE_IMPULSE_STEP = 1.0f;
static const float32 MIN_FORCE_DISTANCE = 4.0f;

static Vector3 BIG_CUBE_CAMERA_OFFSET = Vector3(0.0f, 15.0f, 10.0f);

template <typename T>
bool CompareTransform(const T& lhs, const T& rhs, uint32 size, float32 epsilon, uint32 frameId)
{
    for (uint32 i = 0; i < size; ++i)
    {
        if (!FLOAT_EQUAL_EPS(lhs.data[i], rhs.data[i], epsilon))
        {
            Logger::Debug("Transforms aren't equal, diff: %f, index: %d, frame: %d", std::abs(lhs.data[i] - rhs.data[i]), i, frameId);
            return false;
        }
    }
    return true;
}
}

GameModeSystemPhysics::GameModeSystemPhysics(Scene* scene)
    : INetworkInputSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>() | ComponentUtils::MakeMask<NetworkInputComponent>())
    , scene(scene)
{
    using namespace GameModeSystemPhysicsDetail;

    if (IsServer(this))
    {
        server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
        server->SubscribeOnConnect([this](const Responder& responder)
                                   {
                                       // TODO: Unsubscribe instead of a bool variable. No API to do that right now
                                       static bool began = false;
                                       if (!began)
                                       {
                                           // Remember client id to use when creating entities
                                           NetworkGameModeSingleComponent* networkGameMode = GetScene()->GetSingletonComponent<NetworkGameModeSingleComponent>();
                                           playerId = networkGameMode->GetNetworkPlayerID(responder.GetToken());

                                           this->CreateEntities();

                                           began = true;
                                       }
                                   });
    }

    // Setup camera

    const VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
    const float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    camera = new Camera();
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->RebuildCameraFromValues();
    camera->SetupPerspective(70.f, screenAspect, 0.5f, 2500.f);
    GetScene()->SetCurrentCamera(camera);

    // Setup room bounds

    CreateFloorAndWalls();

    // Setup rendering params

    RenderSystem2D::RenderTargetPassDescriptor targetDesc = GetEngineContext()->renderSystem2D->GetMainTargetDescriptor();
    targetDesc.clearColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
    GetEngineContext()->renderSystem2D->SetMainTargetDescriptor(targetDesc);

    Light* light = new Light();
    light->SetType(Light::eType::TYPE_DIRECTIONAL);
    light->SetDiffuseColor(Color::White);
    light->SetAmbientColor(Color::White);
    light->SetDirection(Vector3(-1.0f, 0.0f, 0.0f));
    light->SetIntensity(100.0f);
    LightComponent* lightComponent = new LightComponent(light);
    Entity* lightEntity = new Entity();
    lightEntity->AddComponent(lightComponent);
    scene->AddNode(lightEntity);

    uint8 whiteTextureData[16 * 16 * 4];
    memset(whiteTextureData, 0xFFFFFF, 16 * 16 * 4);
    cubesTexture = Texture::CreateFromData(FORMAT_RGBA8888, whiteTextureData, 16, 16, true);
    cubesTexture->SetWrapMode(rhi::TEXADDR_WRAP, rhi::TEXADDR_WRAP);

    // Setup input actions

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    uint32 keyboardId = ~0; // TODO: get rid of this
    if (kb != nullptr)
    {
        keyboardId = kb->GetId();
    }

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    actionsSingleComponent->CollectDigitalAction(ACTION_FORWARD, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ACTION_BACK, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ACTION_LEFT, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ACTION_RIGHT, eInputElements::KB_D, keyboardId);

    dynamicBodies = scene->AquireComponentGroup<DynamicBodyComponent, DynamicBodyComponent>();
}

void GameModeSystemPhysics::ProcessFixed(float32 timeElapsed)
{
    using namespace GameModeSystemPhysicsDetail;

    // Update camera position to track the big cube

    if (bigCube != nullptr)
    {
        TransformComponent* cubeTransform = bigCube->GetComponent<TransformComponent>();
        Vector3 cubePosition = cubeTransform->GetPosition();

        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(cubePosition + BIG_CUBE_CAMERA_OFFSET);
        camera->SetTarget(cubePosition);
        camera->RebuildCameraFromValues();
    }

    // Input

    if (bigCube != nullptr)
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), bigCube);
        for (const auto& actions : allActions)
        {
            ApplyDigitalActions(bigCube, actions.digitalActions, actions.clientFrameId, timeElapsed);
        }
    }

    // Physics simulation

    for (Entity* smallCube : smallCubes)
    {
        ScheduleMagneteForces(smallCube);
    }

    // Indicate active actors
    for (DynamicBodyComponent* body : dynamicBodies->components)
    {
        static const Color INACTIVE_CUBE_COLOR = Color::White;
        static const Color ACTIVE_CUBE_COLOR = Color(1.0f, 0.5f, 0.45f, 1.0f);

        bool active = body->GetIsActive();
        Color paint = active ? ACTIVE_CUBE_COLOR : INACTIVE_CUBE_COLOR;
        body->GetEntity()->GetComponent<RenderComponent>()->GetRenderObject()->GetRenderBatch(0)->GetMaterial()->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, paint.color);
    }
}

void GameModeSystemPhysics::PrepareForRemove()
{
}

void GameModeSystemPhysics::Simulate(DAVA::Entity* entity)
{
    DVASSERT(server == nullptr);

    INetworkInputSimulationSystem::Simulate(entity);

    if (entity != bigCube)
    {
        ScheduleMagneteForces(entity);
    }
}

void GameModeSystemPhysics::ApplyDigitalActions(Entity* entity, const Vector<FastName>& actions, uint32 clientFrameId, float32 duration) const
{
    using namespace GameModeSystemPhysicsDetail;

    Vector3 impulse = Vector3::Zero;

    for (const FastName& action : actions)
    {
        if (action == ACTION_FORWARD)
        {
            impulse.y -= BIG_CUBE_IMPULSE_STEP;
        }
        else if (action == ACTION_BACK)
        {
            impulse.y += BIG_CUBE_IMPULSE_STEP;
        }
        else if (action == ACTION_LEFT)
        {
            impulse.x += BIG_CUBE_IMPULSE_STEP;
        }
        else if (action == ACTION_RIGHT)
        {
            impulse.x -= BIG_CUBE_IMPULSE_STEP;
        }
    }

    if (impulse != Vector3::Zero)
    {
        scene->GetSystem<PhysicsSystem>()->AddForce(entity->GetComponent<DynamicBodyComponent>(), impulse, physx::PxForceMode::eIMPULSE);
    }
}

void GameModeSystemPhysics::CreateFloorAndWalls()
{
    using namespace GameModeSystemPhysicsDetail;

    PlaneShapeComponent* planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(WALL_COLLISION_SHAPE_TYPE);
    planeShape->SetTypeMaskToCollideWith(CUBE_COLLISION_SHAPE_TYPE);
    Entity* floorEntity = new Entity();
    floorEntity->AddComponent(new StaticBodyComponent());
    floorEntity->AddComponent(planeShape);
    scene->AddNode(floorEntity);

    planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(WALL_COLLISION_SHAPE_TYPE);
    planeShape->SetTypeMaskToCollideWith(CUBE_COLLISION_SHAPE_TYPE);
    planeShape->SetPoint({ 0.0f, -ROOM_SIZE / 2.0f, 0.0f });
    planeShape->SetNormal({ 0.0f, 1.0, 0.0f });
    Entity* frontWallEntity = new Entity();
    frontWallEntity->AddComponent(new StaticBodyComponent());
    frontWallEntity->AddComponent(planeShape);
    scene->AddNode(frontWallEntity);

    planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(WALL_COLLISION_SHAPE_TYPE);
    planeShape->SetTypeMaskToCollideWith(CUBE_COLLISION_SHAPE_TYPE);
    planeShape->SetPoint({ 0.0f, ROOM_SIZE / 2.0f, 0.0f });
    planeShape->SetNormal({ 0.0f, -1.0, 0.0f });
    Entity* backWallEntity = new Entity();
    backWallEntity->AddComponent(new StaticBodyComponent());
    backWallEntity->AddComponent(planeShape);
    scene->AddNode(backWallEntity);

    planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(WALL_COLLISION_SHAPE_TYPE);
    planeShape->SetTypeMaskToCollideWith(CUBE_COLLISION_SHAPE_TYPE);
    planeShape->SetPoint({ -ROOM_SIZE / 2.0f, 0.0f, 0.0f });
    planeShape->SetNormal({ 1.0f, 0.0, 0.0f });
    Entity* leftWallEntity = new Entity();
    leftWallEntity->AddComponent(new StaticBodyComponent());
    leftWallEntity->AddComponent(planeShape);
    scene->AddNode(leftWallEntity);

    planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(WALL_COLLISION_SHAPE_TYPE);
    planeShape->SetTypeMaskToCollideWith(CUBE_COLLISION_SHAPE_TYPE);
    planeShape->SetPoint({ ROOM_SIZE / 2.0f, 0.0f, 0.0f });
    planeShape->SetNormal({ -1.0f, 0.0, 0.0f });
    Entity* rightWallEntity = new Entity();
    rightWallEntity->AddComponent(new StaticBodyComponent());
    rightWallEntity->AddComponent(planeShape);
    scene->AddNode(rightWallEntity);
}

void GameModeSystemPhysics::CreateEntities()
{
    using namespace GameModeSystemPhysicsDetail;

    DVASSERT(server != nullptr);
    DVASSERT(bigCube == nullptr);
    DVASSERT(smallCubes.size() == 0);

    // Create player-controller cube

    bigCube = CreateBigCube(nullptr);
    bigCube->GetComponent<TransformComponent>()->SetLocalTransform(Vector3(-0.5f, 15.0f, BIG_CUBE_HALF_SIZE), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
    GetScene()->AddNode(bigCube);

    // Create grid of small cubes for interactions

    static const float32 GRID_LEFT_TOP = -static_cast<float32>(SMALL_CUBES_GRID_SIZE) / 2.0f;

    for (uint32 i = 0; i < SMALL_CUBES_GRID_SIZE; ++i)
    {
        for (uint32 j = 0; j < SMALL_CUBES_GRID_SIZE; ++j)
        {
            Vector3 position(GRID_LEFT_TOP + i, GRID_LEFT_TOP + j, SMALL_CUBE_HALF_SIZE);

            Entity* cube = CreateSmallCube(nullptr);
            cube->GetComponent<TransformComponent>()->SetLocalTransform(position, Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
            GetScene()->AddNode(cube);

            smallCubes.insert(cube);
        }
    }
}

void GameModeSystemPhysics::OnEntityReplicated(Entity* entity)
{
    DVASSERT(server == nullptr);

    EntityType replicatedEntityType = entity->GetComponent<NetworkReplicationComponent>()->GetEntityType();
    if (replicatedEntityType == EntityType::SMALL_CUBE)
    {
        CreateSmallCube(entity);
        smallCubes.insert(entity);
    }
    else if (replicatedEntityType == EntityType::BIG_CUBE)
    {
        CreateBigCube(entity);
        bigCube = entity;
    }
}

void GameModeSystemPhysics::ScheduleMagneteForces(Entity* entity)
{
    using namespace GameModeSystemPhysicsDetail;

    if (bigCube == nullptr)
    {
        return;
    }

    const Vector3 smallCubePosition = entity->GetComponent<TransformComponent>()->GetPosition();
    const Vector3 bigCubePosition = bigCube->GetComponent<TransformComponent>()->GetPosition();
    const float32 distanceToBigCube = Distance(bigCubePosition, smallCubePosition);
    if (distanceToBigCube < MIN_FORCE_DISTANCE)
    {
        const Vector3 forceDirection = Normalize(bigCubePosition - smallCubePosition);
        scene->GetSystem<PhysicsSystem>()->AddForce(entity->GetComponent<DynamicBodyComponent>(), forceDirection / distanceToBigCube * 3.0f, physx::PxForceMode::eFORCE);
    }
}

PolygonGroup* GameModeSystemPhysics::CreateCubePolygonGroup(float32 size) const
{
    Array<Vector3, 6 * 4> vertices;
    Array<Vector3, 6 * 4> normals;
    Array<uint16, 6 * 6> indices;
    Array<Vector2, 4> uv;

    uv[0] = Vector2(0.0f, 0.0f);
    uv[1] = Vector2(0.0f, 1.0f);
    uv[2] = Vector2(1.0f, 0.0f);
    uv[3] = Vector2(1.0f, 1.0f);

    normals[0] = normals[1] = normals[2] = normals[3] = Vector3(0.0f, 1.0f, 0.0f);
    vertices[0] = Vector3(size, size, size);
    vertices[1] = Vector3(-size, size, size);
    vertices[2] = Vector3(-size, size, -size);
    vertices[3] = Vector3(size, size, -size);
    indices[0] = 0;
    indices[1] = 3;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 1;

    normals[4] = normals[5] = normals[6] = normals[7] = Vector3(0.0f, -1.0f, 0.0f);
    vertices[4] = Vector3(size, -size, size);
    vertices[5] = Vector3(-size, -size, size);
    vertices[6] = Vector3(-size, -size, -size);
    vertices[7] = Vector3(size, -size, -size);
    indices[6] = 4;
    indices[7] = 6;
    indices[8] = 7;
    indices[9] = 4;
    indices[10] = 5;
    indices[11] = 6;

    normals[8] = normals[9] = normals[10] = normals[11] = Vector3(0.0f, 0.0f, 1.0f);
    vertices[8] = Vector3(size, size, size);
    vertices[9] = Vector3(-size, size, size);
    vertices[10] = Vector3(-size, -size, size);
    vertices[11] = Vector3(size, -size, size);
    indices[12] = 8;
    indices[13] = 10;
    indices[14] = 11;
    indices[15] = 8;
    indices[16] = 9;
    indices[17] = 10;

    normals[12] = normals[13] = normals[14] = normals[15] = Vector3(0.0f, 0.0f, -1.0f);
    vertices[12] = Vector3(size, size, -size);
    vertices[13] = Vector3(-size, size, -size);
    vertices[14] = Vector3(-size, -size, -size);
    vertices[15] = Vector3(size, -size, -size);
    indices[18] = 12;
    indices[19] = 15;
    indices[20] = 14;
    indices[21] = 12;
    indices[22] = 14;
    indices[23] = 13;

    normals[16] = normals[17] = normals[18] = normals[19] = Vector3(1.0f, 0.0f, 0.0f);
    vertices[16] = Vector3(size, -size, size);
    vertices[17] = Vector3(size, size, size);
    vertices[18] = Vector3(size, size, -size);
    vertices[19] = Vector3(size, -size, -size);
    indices[24] = 16;
    indices[25] = 19;
    indices[26] = 18;
    indices[27] = 16;
    indices[28] = 18;
    indices[29] = 17;

    normals[20] = normals[21] = normals[22] = normals[23] = Vector3(-1.0f, 0.0f, 0.0f);
    vertices[20] = Vector3(-size, size, size);
    vertices[21] = Vector3(-size, -size, size);
    vertices[22] = Vector3(-size, -size, -size);
    vertices[23] = Vector3(-size, size, -size);
    indices[30] = 20;
    indices[31] = 23;
    indices[32] = 22;
    indices[33] = 20;
    indices[34] = 22;
    indices[35] = 21;

    PolygonGroup* polygonGroup = new PolygonGroup();
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_TRIANGLELIST);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX | eVertexFormat::EVF_NORMAL | EVF_TEXCOORD0, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), 12);

    for (uint32 i = 0; i < indices.size(); ++i)
    {
        polygonGroup->SetIndex(i, indices[i]);
    }

    for (uint32 i = 0; i < vertices.size(); ++i)
    {
        polygonGroup->SetCoord(i, vertices[i]);
        polygonGroup->SetNormal(i, normals[i]);
        polygonGroup->SetTexcoord(0, i, uv[i % 4]);
    }

    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

Entity* GameModeSystemPhysics::CreateSmallCube(Entity* replicatedGhost) const
{
    using namespace GameModeSystemPhysicsDetail;

    static PolygonGroup* polygonGroup = CreateCubePolygonGroup(SMALL_CUBE_HALF_SIZE);

    NMaterial* material = new NMaterial();
    material->SetMaterialName(FastName("SmallCubeMaterial"));
    material->SetFXName(NMaterialName::VERTEXLIT_OPAQUE);
    material->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, true);
    material->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, cubesTexture);

    DVASSERT(polygonGroup != nullptr);
    DVASSERT(material != nullptr);

    Entity* cube = (replicatedGhost == nullptr) ? new Entity() : replicatedGhost;

    if (replicatedGhost == nullptr)
    {
        NetworkReplicationComponent* networkReplication = new NetworkReplicationComponent();
        networkReplication->SetNetworkPlayerID(playerId);
        networkReplication->SetEntityType(EntityType::SMALL_CUBE);
        cube->AddComponent(networkReplication);

        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        cube->AddComponent(networkTransform);

        cube->AddComponent(new NetworkPlayerComponent());

        cube->AddComponent(new NetworkInputComponent());

        BoxShapeComponent* boxShape = new BoxShapeComponent();
        boxShape->SetHalfSize(Vector3(SMALL_CUBE_HALF_SIZE, SMALL_CUBE_HALF_SIZE, SMALL_CUBE_HALF_SIZE));
        boxShape->SetTypeMaskToCollideWith(WALL_COLLISION_SHAPE_TYPE | CUBE_COLLISION_SHAPE_TYPE);
        boxShape->SetTypeMask(CUBE_COLLISION_SHAPE_TYPE);
        boxShape->SetOverrideMass(true);
        boxShape->SetMass(0.1f);
        cube->AddComponent(boxShape);

        DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
        cube->AddComponent(dynamicBody);
    }
    else
    {
        NetworkPredictComponent* networkPredict = new NetworkPredictComponent();
        cube->AddComponent(networkPredict);
    }

    RenderObject* renderObject = new RenderObject();
    RenderBatch* renderBatch = new RenderBatch();
    renderBatch->SetMaterial(material);
    renderBatch->SetPolygonGroup(polygonGroup);
    renderBatch->vertexLayoutId = static_cast<uint32>(-1);
    renderObject->AddRenderBatch(renderBatch);
    RenderComponent* render = new RenderComponent(renderObject);
    cube->AddComponent(render);

    return cube;
}

Entity* GameModeSystemPhysics::CreateBigCube(Entity* replicatedGhost) const
{
    using namespace GameModeSystemPhysicsDetail;

    PolygonGroup* polygonGroup = CreateCubePolygonGroup(BIG_CUBE_HALF_SIZE);

    NMaterial* material = new NMaterial();
    material->SetMaterialName(FastName("BigCubeMaterial"));
    material->SetFXName(NMaterialName::VERTEXLIT_OPAQUE);
    material->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, true);
    material->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, cubesTexture);

    Entity* cube = (replicatedGhost == nullptr) ? new Entity() : replicatedGhost;

    if (replicatedGhost == nullptr)
    {
        NetworkReplicationComponent* networkReplication = new NetworkReplicationComponent();
        networkReplication->SetNetworkPlayerID(playerId);
        networkReplication->SetEntityType(EntityType::BIG_CUBE);
        cube->AddComponent(networkReplication);

        NetworkTransformComponent* networkTransform = new NetworkTransformComponent();
        cube->AddComponent(networkTransform);

        cube->AddComponent(new NetworkInputComponent());

        BoxShapeComponent* boxShape = new BoxShapeComponent();
        boxShape->SetHalfSize(Vector3(BIG_CUBE_HALF_SIZE, BIG_CUBE_HALF_SIZE, BIG_CUBE_HALF_SIZE));
        boxShape->SetTypeMaskToCollideWith(WALL_COLLISION_SHAPE_TYPE | CUBE_COLLISION_SHAPE_TYPE);
        boxShape->SetTypeMask(CUBE_COLLISION_SHAPE_TYPE);
        boxShape->SetOverrideMass(true);
        boxShape->SetMass(4.0f);
        cube->AddComponent(boxShape);

        DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
        cube->AddComponent(dynamicBody);
    }
    else
    {
        NetworkPredictComponent* networkPredict = new NetworkPredictComponent();
        cube->AddComponent(networkPredict);
    }

    RenderObject* renderObject = new RenderObject();
    RenderBatch* renderBatch = new RenderBatch();
    renderBatch->SetMaterial(material);
    renderBatch->SetPolygonGroup(polygonGroup);
    renderBatch->vertexLayoutId = static_cast<uint32>(-1);
    renderObject->AddRenderBatch(renderBatch);
    RenderComponent* render = new RenderComponent(renderObject);
    cube->AddComponent(render);

    return cube;
}
