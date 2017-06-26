#include "PhysicsDebug/PhysicsDebugDrawSystem.h"

#include <Physics/CollisionShapeComponent.h>
#include <Physics/Private/PhysicsMath.h>

#include <Entity/Component.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Render/3D/PolygonGroup.h>
#include <Math/Color.h>

#include <physx/PxRigidBody.h>
#include <physx/PxShape.h>
#include <physx/geometry/PxBoxGeometry.h>
#include <physx/geometry/PxCapsuleGeometry.h>
#include <physx/geometry/PxSphereGeometry.h>
#include <physx/geometry/PxTriangleMeshGeometry.h>
#include <physx/geometry/PxTriangleMesh.h>
#include <physx/geometry/PxConvexMeshGeometry.h>
#include <physx/geometry/PxConvexMesh.h>
#include <physx/geometry/PxHeightFieldGeometry.h>
#include <physx/geometry/PxHeightField.h>

namespace DAVA
{
namespace PhysicsDebugDrawSystemDetail
{
Array<uint32, 7> collisionShapeTypes = {
    Component::BOX_SHAPE_COMPONENT,
    Component::CAPSULE_SHAPE_COMPONENT,
    Component::SPHERE_SHAPE_COMPONENT,
    Component::PLANE_SHAPE_COMPONENT,
    Component::CONVEX_HULL_SHAPE_COMPONENT,
    Component::MESH_SHAPE_COMPONENT,
    Component::HEIGHT_FIELD_SHAPE_COMPONENT
};

bool IsCollisionComponent(Component* component)
{
    uint32 componentType = component->GetType();
    return std::any_of(collisionShapeTypes.begin(), collisionShapeTypes.end(), [componentType](uint32 type) {
        return type == componentType;
    });
}

RenderObject* CreateRenderObject(const physx::PxBoxGeometry& geometry, uint32 vertexLayoutId)
{
    RenderObject* ro = new RenderObject();
    ScopedPtr<NMaterial> material(new NMaterial());

    material->SetMaterialName(FastName("DebugPhysxMaterial"));
    material->SetFXName(NMaterialName::DEBUG_DRAW_ALPHABLEND);
    material->AddProperty(FastName("color"), Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);

    ScopedPtr<RenderBatch> batch(new RenderBatch());
    ScopedPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_TRIANGLESTRIP);

    Vector3 halfExtents = PhysicsMath::PxVec3ToVector3(geometry.halfExtents);
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

    Array<uint16, 20> indices =
    {
      0, 2, 1, 3, 5, 7, 7, 3, 3, 7, 2, 6, 0, 4, 1, 5, 5, 4, 7, 6
    };

    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), 18);
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(Vector3));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();
    batch->SetPolygonGroup(polygonGroup);
    batch->vertexLayoutId = vertexLayoutId;

    ro->AddRenderBatch(batch);

    return ro;
}
} // namespace PhysicsDebugDrawSystemDetail

PhysicsDebugDrawSystem::PhysicsDebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    rhi::VertexLayout vertexLayout;
    vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vertexLayoutId = rhi::VertexLayout::UniqueId(vertexLayout);
}

void PhysicsDebugDrawSystem::RegisterEntity(Entity* entity)
{
    using namespace PhysicsDebugDrawSystemDetail;

    for (uint32 type : collisionShapeTypes)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            RegisterComponent(entity, entity->GetComponent(type, i));
        }
    }
}

void PhysicsDebugDrawSystem::UnregisterEntity(Entity* entity)
{
    using namespace PhysicsDebugDrawSystemDetail;

    for (uint32 type : collisionShapeTypes)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            UnregisterComponent(entity, entity->GetComponent(type, i));
        }
    }
}

void PhysicsDebugDrawSystem::RegisterComponent(Entity* entity, Component* component)
{
    using namespace PhysicsDebugDrawSystemDetail;

    if (IsCollisionComponent(component))
    {
        pendingComponents.insert(static_cast<CollisionShapeComponent*>(component));
    }
}

void PhysicsDebugDrawSystem::UnregisterComponent(Entity* entity, Component* component)
{
    using namespace PhysicsDebugDrawSystemDetail;

    if (IsCollisionComponent(component))
    {
        auto iter = renderObjects.find(component);
        DVASSERT(iter != renderObjects.end());

        RenderObject* ro = iter->second;
        DVASSERT(ro != nullptr);

        GetScene()->GetRenderSystem()->RemoveFromRender(ro);
    }
}

void PhysicsDebugDrawSystem::Process(float32 timeElapsed)
{
    using namespace PhysicsDebugDrawSystemDetail;
    auto iter = pendingComponents.begin();
    while (iter != pendingComponents.end())
    {
        physx::PxShape* shape = (*iter)->GetPxShape();
        if (shape != nullptr)
        {
            switch (shape->getGeometryType())
            {
            case physx::PxGeometryType::eBOX:
            {
                physx::PxBoxGeometry boxGeom;
                shape->getBoxGeometry(boxGeom);
                RenderObject* ro = CreateRenderObject(boxGeom, vertexLayoutId);
                renderObjects.emplace(*iter, ro);
                GetScene()->GetRenderSystem()->RenderPermanent(ro);
            }
            break;
            default:
                break;
            }

            iter = pendingComponents.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

} // namespace DAVA