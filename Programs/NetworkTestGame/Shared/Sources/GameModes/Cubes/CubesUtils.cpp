#include "GameModes/Cubes/CubesUtils.h"

#include "GameModes/Cubes/SmallCubeComponent.h"

#include <NetworkCore/NetworkCoreUtils.h>

#include <Physics/Core/MeshShapeComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/PlaneShapeComponent.h>
#include <Physics/Core/StaticBodyComponent.h>

#include <Engine/EngineContext.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>

#include <UI/UIControlSystem.h>
#include <Render/Material/NMaterial.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/2D/Systems/RenderSystem2D.h>

void CubesUtils::SetupCubesScene(DAVA::Scene* scene)
{
    using namespace DAVA;

    const VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    const Size2i& physicalSize = vcs->GetPhysicalScreenSize();
    const float32 screenAspect = static_cast<float32>(physicalSize.dx) / static_cast<float32>(physicalSize.dy);

    Camera* camera = scene->GetCurrentCamera();
    camera->SetUp(Vector3(0.f, 0.f, 1.f));
    camera->SetPosition(Vector3(0.f, 60.f, 20.f));
    camera->SetTarget(Vector3(0.f, 0.f, 0.f));
    camera->RebuildCameraFromValues();

    auto createPlanePolygon = [](float32 size)
    {
        PolygonGroup* polygonGroup = new PolygonGroup();
        polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_TRIANGLELIST);
        polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX | eVertexFormat::EVF_NORMAL | EVF_TEXCOORD0, 8, 8, 2);

        uint16 ind[12] = { 0, 2, 3, 0, 1, 2, 0, 3, 2, 0, 2, 1 };
        Vector2 uv[4] = { { 1.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f }, { 1.f, 0.f } };
        Vector3 vert[4] = { { size, size, 0.f }, { -size, size, 0.f }, { -size, -size, 0.f }, { size, -size, 0.f } };

        for (uint32 i = 0; i < 12; ++i)
        {
            polygonGroup->SetIndex(i, ind[i]);
        }

        for (int i = 0; i < 8; ++i)
        {
            polygonGroup->SetCoord(i, vert[i % 4]);
            polygonGroup->SetNormal(i, (i < 4 ? Vector3{ 0.f, 0.f, 1.f } : Vector3{ 0.f, 0.f, -1.f }));
            polygonGroup->SetTexcoord(0, i, uv[i % 4]);
        }

        polygonGroup->BuildBuffers();
        polygonGroup->RecalcAABBox();

        return polygonGroup;
    };

    auto createWhiteTexture = []()
    {
        uint8 textureData[16 * 16 * 4];
        std::fill(std::begin(textureData), std::end(textureData), std::numeric_limits<uint8>::max());
        Texture* texture = Texture::CreateFromData(FORMAT_RGBA8888, textureData, 16, 16, true);
        texture->SetWrapMode(rhi::TEXADDR_WRAP, rhi::TEXADDR_WRAP);
        return texture;
    };

    // Floor.
    PlaneShapeComponent* planeShape = new PlaneShapeComponent();
    planeShape->SetTypeMask(CubesDetails::WallCollisionShapeType);
    planeShape->SetTypeMaskToCollideWith(CubesDetails::CubeCollisionShapeType);
    Entity* floorEntity = new Entity();
    floorEntity->AddComponent(new StaticBodyComponent());
    floorEntity->AddComponent(planeShape);
    scene->AddNode(floorEntity);

    ScopedPtr<Texture> texture(createWhiteTexture());
    ScopedPtr<PolygonGroup> polygon(createPlanePolygon(CubesDetails::RoomSize / 2.0f));

    ScopedPtr<NMaterial> material(new NMaterial());
    material->SetFXName(NMaterialName::VERTEXLIT_OPAQUE);
    material->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, true);
    material->AddProperty(NMaterialParamName::PARAM_FLAT_COLOR, Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);
    material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, texture.get());

    ScopedPtr<RenderObject> renderObject(new RenderObject());
    ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
    renderBatch->SetMaterial(material.get());
    renderBatch->SetPolygonGroup(polygon.get());
    renderObject->AddRenderBatch(renderBatch.get());
    floorEntity->AddComponent(new RenderComponent(renderObject.get()));

    // 4 walls.
    for (const Vector3& normal : Vector<Vector3>{ { 0.0f, 1.0, 0.0f }, { 0.0f, -1.0, 0.0f }, { 1.0f, 0.0, 0.0f }, { -1.0f, 0.0, 0.0f } })
    {
        PlaneShapeComponent* planeShape = new PlaneShapeComponent();
        planeShape->SetTypeMask(CubesDetails::WallCollisionShapeType);
        planeShape->SetTypeMaskToCollideWith(CubesDetails::CubeCollisionShapeType);
        planeShape->SetPoint(normal * (-CubesDetails::RoomSize / 2.0f));
        planeShape->SetNormal(normal);
        Entity* entity = new Entity();
        entity->AddComponent(new StaticBodyComponent());
        entity->AddComponent(planeShape);
        scene->AddNode(entity);
    }

    ScopedPtr<Light> light(new Light());
    light->SetType(Light::eType::TYPE_AMBIENT);
    LightComponent* lightComponent = new LightComponent(light);
    Entity* lightEntity = new Entity();
    lightEntity->GetComponent<TransformComponent>()->SetLocalTransform({ 0.f, 0.f, 50.f }, {}, {});
    lightEntity->AddComponent(lightComponent);
    scene->AddNode(lightEntity);

    if (IsServer(scene))
    {
        FillSceneWithSmallCubes(scene);
    }
}

void CubesUtils::FillSceneWithSmallCubes(DAVA::Scene* scene)
{
    using namespace DAVA;

    const uint32 SmallCubesGridSize = 8;
    const float32 GridLeftTop = -static_cast<float32>(SmallCubesGridSize) / 2.f;

    for (uint32 i = 0; i < SmallCubesGridSize; ++i)
    {
        for (uint32 j = 0; j < SmallCubesGridSize; ++j)
        {
            Vector3 position(GridLeftTop + i, GridLeftTop + j, CubesDetails::SmallCubeHalfSize);

            Entity* cube = new Entity();
            cube->GetComponent<TransformComponent>()->SetLocalTransform(position, {}, Vector3::Zero + 1.f);
            cube->AddComponent(new SmallCubeComponent());
            scene->AddNode(cube);
        }
    }
}
