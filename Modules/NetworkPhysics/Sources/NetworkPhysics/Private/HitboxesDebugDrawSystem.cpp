#include "NetworkPhysics/HitboxesDebugDrawSystem.h"
#include "NetworkPhysics/HitboxesDebugDrawComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Render/Highlevel/RenderObject.h>

#include <Physics/Core/PhysicsUtils.h>
#include <Physics/Core/CollisionShapeComponent.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/PhysicsUtils.h>
#include <Physics/Core/Private/PhysicsMath.h>

#include <physx/extensions/PxShapeExt.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(HitboxesDebugDrawSystem)
{
    ReflectionRegistrator<HitboxesDebugDrawSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &HitboxesDebugDrawSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 0.0f)]
    .End();
}

namespace HitboxesDebugDrawSystemDetail
{
static const Color CLIENT_HITBOXES_COLOR = Color(0.0f, 1.0f, 0.0f, 1.0f);
static const Color SERVER_HITBOXES_COLOR = Color(1.0f, 1.0f, 1.0f, 1.0f);
static const float32 SERVER_HITBOXES_SIZE_DELTA = -0.005f;

template <typename TVertices, typename TIndices>
RefPtr<PolygonGroup> AllocatePolygonGroup(const TVertices& vertices, const TIndices& indices, const rhi::PrimitiveType primitiveType, const int32 primitiveCount)
{
    RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(primitiveType);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), primitiveCount);
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

RefPtr<PolygonGroup> CreateBoxPolygonGroup(const physx::PxBoxGeometry& geom, const Vector3 sizeDelta)
{
    Vector3 halfExtents = PhysicsMath::PxVec3ToVector3(geom.halfExtents);
    halfExtents += sizeDelta;

    Array<Vector3, 8> vertices =
    {
      Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
      Vector3(halfExtents.x, -halfExtents.y, -halfExtents.z),
      Vector3(-halfExtents.x, halfExtents.y, -halfExtents.z),
      Vector3(halfExtents.x, halfExtents.y, -halfExtents.z),
      Vector3(-halfExtents.x, -halfExtents.y, halfExtents.z),
      Vector3(halfExtents.x, -halfExtents.y, halfExtents.z),
      Vector3(-halfExtents.x, halfExtents.y, halfExtents.z),
      Vector3(halfExtents.x, halfExtents.y, halfExtents.z),
    };

    Array<uint16, 24> indices =
    {
      0, 1,
      1, 3,
      3, 2,
      2, 0,
      4, 5,
      5, 7,
      7, 6,
      6, 4,
      0, 4,
      2, 6,
      3, 7,
      1, 5
    };

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_LINELIST, 12);
}
}

HitboxesDebugDrawSystem::HitboxesDebugDrawSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask())
    , hitboxesDebugDrawComponents(scene->AquireComponentGroup<HitboxesDebugDrawComponent, HitboxesDebugDrawComponent>())
{
    rhi::VertexLayout vertexLayout;
    vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    renderObjectsVertexLayoutId = rhi::VertexLayout::UniqueId(vertexLayout);
}

HitboxesDebugDrawSystem::~HitboxesDebugDrawSystem()
{
    Scene* scene = GetScene();
    DVASSERT(scene != nullptr);

    RenderSystem* renderSystem = scene->GetRenderSystem();
    DVASSERT(renderSystem != nullptr);

    for (auto& kvp : hitboxRenderObjectsMap)
    {
        HitboxRenderObjects& renderObjects = kvp.second;

        if (renderObjects.roClient != nullptr)
        {
            renderSystem->RemoveFromRender(renderObjects.roClient);
            SafeRelease(renderObjects.roClient);
        }

        if (renderObjects.roServer != nullptr)
        {
            renderSystem->RemoveFromRender(renderObjects.roServer);
            SafeRelease(renderObjects.roServer);
        }
    }

    hitboxRenderObjectsMap.clear();
}

void HitboxesDebugDrawSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("HitboxesDebugDrawSystem::ProcessFixed");

    for (HitboxesDebugDrawComponent* component : hitboxesDebugDrawComponents->components)
    {
        DVASSERT(component != nullptr);
        UpdateRenderObjects(*component);
    }
}

void HitboxesDebugDrawSystem::UpdateRenderObjects(HitboxesDebugDrawComponent& hitboxesDebugDrawComponent)
{
    using namespace HitboxesDebugDrawSystemDetail;

    Entity* entity = hitboxesDebugDrawComponent.GetEntity();
    DVASSERT(entity != nullptr);

    // Collect all collision shape components
    CollisionShapeComponent* shapes[HitboxesDebugDrawComponent::NumMaxHitboxes];
    uint32 numShapes = 0;
    PhysicsUtils::ForEachShapeComponent(entity, [&shapes, &numShapes](CollisionShapeComponent* shape)
                                        {
                                            if (numShapes < HitboxesDebugDrawComponent::NumMaxHitboxes)
                                            {
                                                shapes[numShapes] = shape;
                                                ++numShapes;
                                            }
                                        });

    if (hitboxesDebugDrawComponent.clientHitboxPositions.size() > 0)
    {
        DVASSERT(hitboxesDebugDrawComponent.clientHitboxOrientations.size() == hitboxesDebugDrawComponent.clientHitboxPositions.size());
        DVASSERT(numShapes == hitboxesDebugDrawComponent.clientHitboxPositions.size());

        for (uint32 i = 0; i < numShapes; ++i)
        {
            const CollisionShapeComponent* shapeComponent = shapes[i];
            DVASSERT(shapeComponent != nullptr);

            HitboxRenderObjects& hitboxRenderObjects = hitboxRenderObjectsMap[shapeComponent];

            // If RO isn't created yet, create it
            if (hitboxRenderObjects.roClient == nullptr)
            {
                hitboxRenderObjects.roClient = CreateHitboxRenderObject(
                *shapeComponent,
                hitboxRenderObjects.roClientWorldTransform,
                CLIENT_HITBOXES_COLOR,
                Vector3(SERVER_HITBOXES_SIZE_DELTA, SERVER_HITBOXES_SIZE_DELTA, SERVER_HITBOXES_SIZE_DELTA));
            }

            DVASSERT(hitboxRenderObjects.roClient != nullptr);

            const Matrix4 newWorldTransform = Matrix4(hitboxesDebugDrawComponent.clientHitboxPositions[i], hitboxesDebugDrawComponent.clientHitboxOrientations[i], Vector3(1.0f, 1.0f, 1.0f));
            if (newWorldTransform != hitboxRenderObjects.roClientWorldTransform)
            {
                hitboxRenderObjects.roClientWorldTransform = newWorldTransform;
                GetScene()->GetRenderSystem()->MarkForUpdate(hitboxRenderObjects.roClient);
            }
        }
    }

    if (hitboxesDebugDrawComponent.serverHitboxPositions.size() > 0)
    {
        DVASSERT(hitboxesDebugDrawComponent.serverHitboxOrientations.size() == hitboxesDebugDrawComponent.serverHitboxPositions.size());
        DVASSERT(numShapes == hitboxesDebugDrawComponent.serverHitboxPositions.size());

        for (uint32 i = 0; i < numShapes; ++i)
        {
            const CollisionShapeComponent* shapeComponent = shapes[i];
            DVASSERT(shapeComponent != nullptr);

            HitboxRenderObjects& hitboxRenderObjects = hitboxRenderObjectsMap[shapeComponent];

            // If RO isn't created yet, create it
            if (hitboxRenderObjects.roServer == nullptr)
            {
                hitboxRenderObjects.roServer = CreateHitboxRenderObject(
                *shapeComponent,
                hitboxRenderObjects.roServerWorldTransform,
                SERVER_HITBOXES_COLOR,
                Vector3::Zero);
            }

            DVASSERT(hitboxRenderObjects.roServer != nullptr);

            const Matrix4 newWorldTransform = Matrix4(hitboxesDebugDrawComponent.serverHitboxPositions[i], hitboxesDebugDrawComponent.serverHitboxOrientations[i], Vector3(1.0f, 1.0f, 1.0f));
            if (newWorldTransform != hitboxRenderObjects.roServerWorldTransform)
            {
                hitboxRenderObjects.roServerWorldTransform = newWorldTransform;
                GetScene()->GetRenderSystem()->MarkForUpdate(hitboxRenderObjects.roServer);
            }
        }
    }
}

RenderObject* HitboxesDebugDrawSystem::CreateHitboxRenderObject(const CollisionShapeComponent& shape, Matrix4& worldTransform, Color color, Vector3 boxesDelta)
{
    using namespace HitboxesDebugDrawSystemDetail;

    // Only draw boxes for now
    DVASSERT(shape.GetType()->Is<BoxShapeComponent>());

    physx::PxShape* pxShape = shape.GetPxShape();
    DVASSERT(pxShape != nullptr);

    RenderObject* ro = new RenderObject();

    RefPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName("HitboxesDebugDrawSystemMaterial"));
    material->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);
    material->AddProperty(FastName("color"), color.color, rhi::ShaderProp::TYPE_FLOAT4);

    RefPtr<PolygonGroup> polygonGroup = CreateBoxPolygonGroup(pxShape->getGeometry().box(), boxesDelta);
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    batch->SetMaterial(material.Get());
    batch->SetPolygonGroup(polygonGroup.Get());
    batch->vertexLayoutId = renderObjectsVertexLayoutId;
    ro->AddRenderBatch(batch);

    ro->SetWorldMatrixPtr(&worldTransform);

    GetScene()->GetRenderSystem()->RenderPermanent(ro);

    return ro;
}

} //namespace DAVA
