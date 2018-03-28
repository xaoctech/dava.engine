#include "GameModes/Cubes/CubesEntityFillSystem.h"

#include "GameModes/Cubes/CubesUtils.h"

#include "GameModes/Cubes/BigCubeComponent.h"
#include "GameModes/Cubes/SmallCubeComponent.h"

#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPlayerComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkPredictComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkRemoteInputComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>

#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>

#include <physx/PxRigidDynamic.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Texture.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Render/Material/NMaterial.h>
#include <Render/Highlevel/RenderObject.h>

DAVA_VIRTUAL_REFLECTION_IMPL(CubesEntityFillSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<CubesEntityFillSystem>::Begin()[M::Tags("gm_cubes")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &CubesEntityFillSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 25.2f)]
    .End();
}

CubesEntityFillSystem::CubesEntityFillSystem(DAVA::Scene* scene)
    : DAVA::BaseSimulationSystem(scene, DAVA::ComponentMask())
    , pendingBigCubes(scene->AquireEntityGroupOnAdd(scene->AquireEntityGroup<BigCubeComponent>(), this))
    , pendingSmallCubes(scene->AquireEntityGroupOnAdd(scene->AquireEntityGroup<SmallCubeComponent>(), this))
    , dynamicBodies(scene->AquireComponentGroup<DAVA::DynamicBodyComponent, DAVA::DynamicBodyComponent>())
{
    using namespace DAVA;

    uint8 whiteTextureData[16 * 16 * 4];
    std::fill(std::begin(whiteTextureData), std::end(whiteTextureData), std::numeric_limits<uint8>::max());
    cubeTexture = Texture::CreateFromData(FORMAT_RGBA8888, whiteTextureData, 16, 16, true);
    cubeTexture->SetWrapMode(rhi::TEXADDR_WRAP, rhi::TEXADDR_WRAP);

    smallCubePolygonGroup = CreateCubePolygonGroup(CubesDetails::SmallCubeHalfSize);
    bigCubePolygonGroup = CreateCubePolygonGroup(CubesDetails::BigCubeHalfSize);
}

CubesEntityFillSystem::~CubesEntityFillSystem()
{
    DAVA::SafeRelease(smallCubePolygonGroup);
    DAVA::SafeRelease(bigCubePolygonGroup);
}

void CubesEntityFillSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (DAVA::Entity* bigCube : std::exchange(pendingBigCubes->entities, {}))
    {
        FillBigCube(bigCube);
    }

    for (DAVA::Entity* smallCube : std::exchange(pendingSmallCubes->entities, {}))
    {
        FillSmallCube(smallCube);
    }

    UpdateBodiesColors(IsReSimulating() ? 0.f : timeElapsed);
}

void CubesEntityFillSystem::FillRenderObject(DAVA::Entity* cube, DAVA::PolygonGroup* polygonGroup, const DAVA::Color& color) const
{
    using namespace DAVA;

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetFXName(NMaterialName::VERTEXLIT_OPAQUE);
    material->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, true);
    material->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, color.color, rhi::ShaderProp::TYPE_FLOAT4);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, cubeTexture);

    ScopedPtr<RenderObject> renderObject(new RenderObject());
    ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
    renderBatch->SetMaterial(material.get());
    renderBatch->SetPolygonGroup(polygonGroup);
    renderObject->AddRenderBatch(renderBatch.get());
    cube->AddComponent(new RenderComponent(renderObject.get()));
}

void CubesEntityFillSystem::FillBigCube(DAVA::Entity* bigCube) const
{
    using namespace DAVA;

    FillRenderObject(bigCube, bigCubePolygonGroup, bigCubeColor);

    if (IsServer(GetScene()))
    {
        BigCubeComponent* bigCubeComponent = bigCube->GetComponent<BigCubeComponent>();

        DVASSERT(bigCubeComponent != nullptr);

        NetworkPlayerID playerId = bigCubeComponent->playerId;

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(NetworkID::CreatePlayerOwnId(playerId));

        replicationComponent->SetForReplication<BigCubeComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<DynamicBodyComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkInputComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkPlayerComponent>(M::Privacy::PRIVATE);
        replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkRemoteInputComponent>(M::Privacy::PUBLIC);

        bigCube->AddComponent(replicationComponent);
        bigCube->AddComponent(new DynamicBodyComponent());
        bigCube->AddComponent(new NetworkInputComponent());
        bigCube->AddComponent(new NetworkPlayerComponent());
        bigCube->AddComponent(new NetworkTransformComponent());

        NetworkRemoteInputComponent* remoteInputComponent = new NetworkRemoteInputComponent();

        using namespace CubesDetails;

        for (const FastName& action : { ActionForward, ActionBack, ActionLeft, ActionRight })
        {
            remoteInputComponent->AddActionToReplicate(action);
        }

        bigCube->AddComponent(remoteInputComponent);

        bigCube->AddComponent(new ObserverComponent());

        bigCube->AddComponent(new ObservableComponent());
        bigCube->AddComponent(new SimpleVisibilityShapeComponent());
    }

    BoxShapeComponent* boxShape = new BoxShapeComponent();
    boxShape->SetHalfSize(Vector3::Zero + CubesDetails::BigCubeHalfSize);
    boxShape->SetTypeMask(CubesDetails::CubeCollisionShapeType);
    boxShape->SetTypeMaskToCollideWith(CubesDetails::WallCollisionShapeType | CubesDetails::CubeCollisionShapeType);
    boxShape->SetOverrideMass(true);
    boxShape->SetMass(4.0f);
    bigCube->AddComponent(boxShape);

    if (IsClient(GetScene()))
    {
        ComponentMask predictionMask;
        predictionMask.Set<DynamicBodyComponent>();
        predictionMask.Set<NetworkTransformComponent>();

        NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionMask);
        bigCube->AddComponent(networkPredictComponent);

        bigCube->SetName(IsClientOwner(bigCube) ? "Me" : "Some friendly big cube");
    }
}

void CubesEntityFillSystem::FillSmallCube(DAVA::Entity* smallCube) const
{
    using namespace DAVA;

    FillRenderObject(smallCube, smallCubePolygonGroup, smallCubeColor);

    if (IsServer(GetScene()))
    {
        DVASSERT(smallCube->GetComponent<SmallCubeComponent>() != nullptr);

        NetworkReplicationComponent* replicationComponent = new NetworkReplicationComponent(NetworkID::CreatePlayerOwnId(0));

        replicationComponent->SetForReplication<SmallCubeComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<DynamicBodyComponent>(M::Privacy::PUBLIC);
        replicationComponent->SetForReplication<NetworkTransformComponent>(M::Privacy::PUBLIC);

        smallCube->AddComponent(replicationComponent);
        smallCube->AddComponent(new DynamicBodyComponent());
        smallCube->AddComponent(new NetworkTransformComponent());

        smallCube->AddComponent(new ObservableComponent());
        smallCube->AddComponent(new SimpleVisibilityShapeComponent());
    }

    BoxShapeComponent* boxShape = new BoxShapeComponent();
    boxShape->SetHalfSize(Vector3::Zero + CubesDetails::SmallCubeHalfSize);
    boxShape->SetTypeMask(CubesDetails::CubeCollisionShapeType);
    boxShape->SetTypeMaskToCollideWith(CubesDetails::WallCollisionShapeType | CubesDetails::CubeCollisionShapeType);
    boxShape->SetOverrideMass(true);
    boxShape->SetMass(0.1f);
    smallCube->AddComponent(boxShape);

    if (IsClient(GetScene()))
    {
        ComponentMask predictionMask;
        predictionMask.Set<DynamicBodyComponent>();
        predictionMask.Set<NetworkTransformComponent>();

        NetworkPredictComponent* networkPredictComponent = new NetworkPredictComponent(predictionMask);
        smallCube->AddComponent(networkPredictComponent);
    }

    smallCube->SetName("Small cube");
}

DAVA::PolygonGroup* CubesEntityFillSystem::CreateCubePolygonGroup(DAVA::float32 size) const
{
    using namespace DAVA;

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

void CubesEntityFillSystem::UpdateBodiesColors(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    for (DynamicBodyComponent* body : dynamicBodies->components)
    {
        float32& time = timeInfo[body];

        physx::PxRigidActor* rigidActor = body->GetPxActor();
        physx::PxRigidDynamic* rigidDynamic = rigidActor != nullptr ? rigidActor->is<physx::PxRigidDynamic>() : nullptr;

        if (rigidDynamic != nullptr && rigidDynamic->getWakeCounter() != 0)
        {
            time = 0.f;
        }
        else if (time < fadeTime)
        {
            time += timeElapsed;
        }

        Entity* entity = body->GetEntity();

        DVASSERT(entity != nullptr);

        NMaterial* material = entity->GetComponent<RenderComponent>()->GetRenderObject()->GetActiveRenderBatch(0)->GetMaterial();

        DVASSERT(material != nullptr);

        bool isSmallCube = entity->GetComponent<SmallCubeComponent>() != nullptr;

        Color color = isSmallCube ? smallCubeColor : bigCubeColor;

        if (time < fadeTime)
        {
            float32 d = time / fadeTime;
            color.r *= d;
            color.g = Clamp(Max(color.g, 1.f - time / fadeTime), 0.f, 1.f);
            color.b *= d;
        }

        material->SetPropertyValue(NMaterialParamName::PARAM_FLAT_COLOR, color.color);
    }
}
