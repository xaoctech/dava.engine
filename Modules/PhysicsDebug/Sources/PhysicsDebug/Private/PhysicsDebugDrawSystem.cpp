#include "PhysicsDebug/PhysicsDebugDrawSystem.h"

#include <Physics/CollisionShapeComponent.h>
#include <Physics/Private/PhysicsMath.h>

#include <Entity/Component.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
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

const uint16 VERTEX_PER_CIRCLE = 32;

bool IsGeometryEqual(const physx::PxGeometryHolder& holder1, const physx::PxGeometryHolder& holder2)
{
    if (holder1.getType() != holder2.getType())
    {
        return false;
    }

    bool result = false;
    switch (holder1.getType())
    {
    case physx::PxGeometryType::eBOX:
    {
        physx::PxBoxGeometry box1 = holder1.box();
        physx::PxBoxGeometry box2 = holder2.box();

        if (box1.halfExtents == box2.halfExtents)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eSPHERE:
    {
        physx::PxSphereGeometry sphere1 = holder1.sphere();
        physx::PxSphereGeometry sphere2 = holder2.sphere();

        if (sphere1.radius == sphere2.radius)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eCAPSULE:
    {
        physx::PxCapsuleGeometry capsule1 = holder1.capsule();
        physx::PxCapsuleGeometry capsule2 = holder2.capsule();

        if (capsule1.radius == capsule2.radius &&
            capsule1.halfHeight == capsule2.halfHeight)
        {
            result = true;
        }
    }
    break;
    default:
        DVASSERT(false);
        break;
    }

    return result;
}

bool IsCollisionComponent(Component* component)
{
    uint32 componentType = component->GetType();
    return std::any_of(collisionShapeTypes.begin(), collisionShapeTypes.end(), [componentType](uint32 type) {
        return type == componentType;
    });
}

void GenerateArc(float32 startRad, float32 endRad, float32 radius, const Vector3& center,
                 const Vector3& direction, bool closeArc, const Matrix4& localPose,
                 Vector<Vector3>& vertices, Vector<uint16>& indices)
{
    float32 deltaRad = endRad - startRad;
    uint16 pointsCount = std::min(static_cast<uint16>(128), static_cast<uint16>(VERTEX_PER_CIRCLE * deltaRad * radius / PI_2));

    const Vector3 dir = Normalize(direction);
    const Vector3 ortho = Abs(dir.x) < Abs(dir.y) ? dir.CrossProduct(Vector3(1.f, 0.f, 0.f)) : dir.CrossProduct(Vector3(0.f, 1.f, 0.f));

    uint16 startVertex = static_cast<uint16>(vertices.size());

    Matrix4 rotationMx;
    float32 angleDelta = deltaRad / pointsCount;
    float32 currentAngle = startRad;
    while (currentAngle < endRad + 0.01)
    {
        rotationMx.CreateRotation(direction, currentAngle);
        uint16 vertexIndex = static_cast<uint16>(vertices.size());
        vertices.push_back(center + ((ortho * radius) * rotationMx) * localPose);

        if (currentAngle > startRad)
        {
            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
        }
        currentAngle += angleDelta;
    }

    if (closeArc == true)
    {
        uint16 endVertexIndex = static_cast<uint16>(vertices.size() - 1);
        indices.push_back(endVertexIndex);
        indices.push_back(startVertex);
    }
}

RefPtr<PolygonGroup> CreateBoxPolygonGroup(const physx::PxBoxGeometry& geom, const Matrix4& localPose)
{
    Vector3 halfExtents = PhysicsMath::PxVec3ToVector3(geom.halfExtents);
    Array<Vector3, 8> vertices =
    {
      Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z) * localPose,
      Vector3(halfExtents.x, -halfExtents.y, -halfExtents.z) * localPose,
      Vector3(-halfExtents.x, halfExtents.y, -halfExtents.z) * localPose,
      Vector3(halfExtents.x, halfExtents.y, -halfExtents.z) * localPose,
      Vector3(-halfExtents.x, -halfExtents.y, halfExtents.z) * localPose,
      Vector3(halfExtents.x, -halfExtents.y, halfExtents.z) * localPose,
      Vector3(-halfExtents.x, halfExtents.y, halfExtents.z) * localPose,
      Vector3(halfExtents.x, halfExtents.y, halfExtents.z) * localPose,
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

    RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_LINELIST);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), 12);
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

RefPtr<PolygonGroup> CreateSpherePolygonGroup(const physx::PxSphereGeometry& geom, const Matrix4& localPose)
{
    uint16 pointsCount = std::min(128u, static_cast<uint32>(geom.radius * VERTEX_PER_CIRCLE));
    Vector<Vector3> vertices;
    Vector<uint16> indices;
    vertices.reserve(pointsCount * 3); // 3 circles per sphere
    indices.reserve(pointsCount * 2 * 3); // 3 circles per sphere, 2 indices per single line

    Array<Vector3, 3> directions = {
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 1.0f)
    };

    for (size_t dirIndex = 0; dirIndex < directions.size(); ++dirIndex)
    {
        GenerateArc(0.0, PI_2, geom.radius, Vector3(0.0f, 0.0f, 0.0f), directions[dirIndex], true, localPose, vertices, indices);
    }

    DVASSERT(static_cast<uint32>(indices.size()) == pointsCount * 2 * 3);

    RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_LINELIST);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), static_cast<int32>(indices.size() >> 1));
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

RefPtr<PolygonGroup> CreateCapsulePolygonGroup(const physx::PxCapsuleGeometry& geom, const Matrix4& localPose)
{
    uint16 pointsCount = std::min(128u, static_cast<uint32>(geom.radius * 32));
    Vector<Vector3> vertices;
    Vector<uint16> indices;
    vertices.reserve(8 + 2 * pointsCount);
    indices.reserve(8 + 2 * 2 * pointsCount);

    Vector3 xAxisOffset = Vector3(1.0f, 0.0f, 0.0f) * geom.halfHeight;
    Vector3 yAxisOffset = Vector3(0.0f, geom.radius, 0.0f);
    Vector3 zAxisOffset = Vector3(0.0f, 0.0f, geom.radius);

    GenerateArc(0.0f, PI_2, geom.radius, xAxisOffset, Vector3(1.0f, 0.0f, 0.0f), true, localPose, vertices, indices);
    GenerateArc(0.0f, PI, geom.radius, xAxisOffset, Vector3(0.0f, 1.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI_05, PI + PI_05, geom.radius, xAxisOffset, Vector3(0.0f, 0.0f, 1.0f), false, localPose, vertices, indices);

    GenerateArc(0.0f, PI_2, geom.radius, -xAxisOffset, Vector3(1.0f, 0.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI, PI_2, geom.radius, -xAxisOffset, Vector3(0.0f, 1.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI + PI_05, PI_2 + PI_05, geom.radius, -xAxisOffset, Vector3(0.0f, 0.0f, 1.0f), false, localPose, vertices, indices);

    uint16 firstLineVertex = static_cast<uint16>(vertices.size());

    vertices.push_back(-xAxisOffset + yAxisOffset);
    vertices.push_back(xAxisOffset + yAxisOffset);
    vertices.push_back(-xAxisOffset - yAxisOffset);
    vertices.push_back(xAxisOffset - yAxisOffset);
    vertices.push_back(-xAxisOffset + zAxisOffset);
    vertices.push_back(xAxisOffset + zAxisOffset);
    vertices.push_back(-xAxisOffset - zAxisOffset);
    vertices.push_back(xAxisOffset - zAxisOffset);

    for (uint16 i = 0; i < 8; ++i)
    {
        indices.push_back(firstLineVertex + i);
    }

    RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_LINELIST);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), static_cast<int32>(indices.size() >> 1));
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

RefPtr<PolygonGroup> CreatePolygonGroup(const physx::PxGeometryHolder& geom, const Matrix4& localPose)
{
    RefPtr<PolygonGroup> result;
    switch (geom.getType())
    {
    case physx::PxGeometryType::eBOX:
        result = CreateBoxPolygonGroup(geom.box(), localPose);
        break;
    case physx::PxGeometryType::eSPHERE:
        result = CreateSpherePolygonGroup(geom.sphere(), localPose);
        break;
    case physx::PxGeometryType::eCAPSULE:
        result = CreateCapsulePolygonGroup(geom.capsule(), localPose);
        break;
    default:
        break;
    }

    return result;
}

RefPtr<NMaterial> CreateMaterial()
{
    RefPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName("DebugPhysxMaterial"));
    material->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);
    material->AddProperty(FastName("color"), Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);

    return material;
}

RenderObject* CreateRenderObject(const physx::PxGeometryHolder& geometry, uint32 vertexLayoutId, const Matrix4& localPose)
{
    ScopedPtr<RenderBatch> batch(new RenderBatch());
    batch->SetMaterial(CreateMaterial().Get());
    batch->SetPolygonGroup(CreatePolygonGroup(geometry, localPose).Get());
    batch->vertexLayoutId = vertexLayoutId;

    RenderObject* ro = new RenderObject();
    ro->AddRenderBatch(batch);

    return ro;
}

} // namespace PhysicsDebugDrawSystemDetail

uint32 PhysicsDebugDrawSystem::vertexLayoutId = static_cast<uint32>(-1);

PhysicsDebugDrawSystem::PhysicsDebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    if (vertexLayoutId == static_cast<uint32>(-1))
    {
        rhi::VertexLayout vertexLayout;
        vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
        vertexLayoutId = rhi::VertexLayout::UniqueId(vertexLayout);
    }
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
        auto iter = renderObjects.find(static_cast<CollisionShapeComponent*>(component));
        if (iter != renderObjects.end())
        {
            RenderObjectInfo& roInfo = iter->second;
            DVASSERT(roInfo.ro != nullptr);

            GetScene()->GetRenderSystem()->RemoveFromRender(roInfo.ro);
            DAVA::SafeRelease(roInfo.ro);
            renderObjects.erase(iter);
        }

        pendingComponents.erase(static_cast<CollisionShapeComponent*>(component));
    }
}

void PhysicsDebugDrawSystem::Process(float32 timeElapsed)
{
    using namespace PhysicsDebugDrawSystemDetail;
    auto iter = pendingComponents.begin();
    while (iter != pendingComponents.end())
    {
        RenderObject* ro = nullptr;
        physx::PxShape* shape = (*iter)->GetPxShape();
        if (shape != nullptr)
        {
            physx::PxGeometryHolder holder = shape->getGeometry();
            ro = CreateRenderObject(holder, vertexLayoutId, PhysicsMath::PxMat44ToMatrix4(physx::PxMat44(shape->getLocalPose())));
            RenderObjectInfo roInfo;
            roInfo.ro = ro;
            roInfo.geomHolder = holder;
            roInfo.localPose = shape->getLocalPose();
            renderObjects.emplace(*iter, roInfo);

            if (ro != nullptr)
            {
                Entity* entity = (*iter)->GetEntity();
                Matrix4* worldTransformPointer = (static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
                ro->SetWorldTransformPtr(worldTransformPointer);
                GetScene()->GetRenderSystem()->RenderPermanent(ro);
            }

            iter = pendingComponents.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    Vector<CollisionShapeComponent*> updateRenderComponents;
    updateRenderComponents.reserve(renderObjects.size());

    for (auto& node : renderObjects)
    {
        const physx::PxGeometryHolder& holder1 = node.second.geomHolder;
        physx::PxShape* shape = node.first->GetPxShape();
        physx::PxTransform localPose = shape->getLocalPose();
        const physx::PxGeometryHolder& holder2 = shape->getGeometry();
        if (IsGeometryEqual(holder1, holder2) == false ||
            !(node.second.localPose.p == localPose.p &&
              node.second.localPose.q == localPose.q))
        {
            updateRenderComponents.push_back(node.first);
        }
        else
        {
            CollisionShapeComponent* shapeComponent = node.first;
            NMaterial* material = node.second.ro->GetRenderBatch(0)->GetMaterial();
            physx::PxShape* shape = shapeComponent->GetPxShape();
            physx::PxActor* actor = shape->getActor();
            if (actor == nullptr)
            {
                material->SetPropertyValue(FastName("color"), Color::White.color);
            }
            else
            {
                if (actor->is<physx::PxRigidStatic>())
                {
                    material->SetPropertyValue(FastName("color"), Color::Red.color);
                }
                else
                {
                    physx::PxRigidDynamic* dynamic = actor->is<physx::PxRigidDynamic>();
                    if (dynamic != nullptr)
                    {
                        if (dynamic->isSleeping())
                        {
                            material->SetPropertyValue(FastName("color"), Color::Yellow.color);
                        }
                        else
                        {
                            material->SetPropertyValue(FastName("color"), Color::Green.color);
                        }
                    }
                }
            }
        }
    }

    for (CollisionShapeComponent* component : updateRenderComponents)
    {
        Entity* e = component->GetEntity();
        UnregisterComponent(e, component);
        RegisterComponent(e, component);
    }

    UnorderedMap<Entity*, RenderObject*> mapping;
    for (auto& node : renderObjects)
    {
        mapping.emplace(node.first->GetEntity(), node.second.ro);
    }

    TransformSingleComponent* trSingle = GetScene()->transformSingleComponent;
    if (trSingle != nullptr)
    {
        for (auto& pair : trSingle->worldTransformChanged.map)
        {
            uint64 components = pair.first->GetComponentsFlags();
            for (uint32 type : collisionShapeTypes)
            {
                if ((components & type) == type)
                {
                    for (Entity* e : pair.second)
                    {
                        auto roIter = mapping.find(e);
                        DVASSERT(roIter != mapping.end());

                        Matrix4* worldTransformPointer = (static_cast<TransformComponent*>(e->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
                        roIter->second->SetWorldTransformPtr(worldTransformPointer);
                    }
                }
            }
        }
    }
}

} // namespace DAVA