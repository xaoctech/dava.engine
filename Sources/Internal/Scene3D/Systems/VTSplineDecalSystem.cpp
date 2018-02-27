#include "Scene3D/Systems/VTSplineDecalSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Render/Highlevel/VTDecalPageRenderer.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"
#include "Scene3D/Components/SplineComponent.h"

namespace DAVA
{
VTSplineDecalSystem::VTSplineDecalSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<VTDecalComponent>() | ComponentUtils::MakeMask<SplineComponent>())
{
}

void VTSplineDecalSystem::Process(float32 timeElapsed)
{
    VTSingleComponent* vtc = GetScene()->GetSingletonComponent<VTSingleComponent>();
    if (vtc)
    {
        for (Entity* entity : vtc->vtDecalChanged)
        {
            VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();
            decalComponent->renderObject->SetDecalSize(decalComponent->decalSize);
            decalComponent->renderObject->SetMaterial(decalComponent->material.Get());
            decalComponent->renderObject->SetSortingOffset(decalComponent->sortingOffset);
            GetScene()->renderSystem->MarkForUpdate(decalComponent->renderObject);
        }
        for (Entity* entity : vtc->vtSplineChanged) //we need to recompute bbox and update it in vt hierarchy
        {
            VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();
            SplineComponent* splineComponent = entity->GetComponent<SplineComponent>();
            if (decalComponent && splineComponent)
            {
                RebuildDecalGeometryData(decalComponent, splineComponent);
                decalComponent->GetRenderObject()->SplineDataUpdated();
            }
        }
    }
}

void VTSplineDecalSystem::AddEntity(Entity* entity)
{
    VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();
    SplineComponent* splineComponent = entity->GetComponent<SplineComponent>();
    DVASSERT(decalComponent);
    DVASSERT(splineComponent);

    DecalRenderObject::SplineRenderData* splineRenderData = new DecalRenderObject::SplineRenderData();
    decalComponent->GetRenderObject()->SetSplineData(splineRenderData);
    RebuildDecalGeometryData(decalComponent, splineComponent);
    decalComponent->GetRenderObject()->SplineDataUpdated();
}
void VTSplineDecalSystem::RemoveEntity(Entity* entity)
{
    VTDecalComponent* component = entity->GetComponent<VTDecalComponent>();
    if (component && component->GetRenderObject())
    {
        DecalRenderObject* decalObject = component->GetRenderObject();
        decalObject->SetSplineData(nullptr);
    }
}

void VTSplineDecalSystem::RebuildDecalGeometryData(VTDecalComponent* decalComponent, SplineComponent* splineComponent)
{
    DecalRenderObject::SplineRenderData* splineRenderData = decalComponent->GetRenderObject()->GetSplineData();

    //GFX_COMPLETE for now we force full update - so clear it
    rhi::DeleteIndexBuffer(splineRenderData->indexBuffer);
    rhi::DeleteVertexBuffer(splineRenderData->vertexBuffer);
    splineRenderData->resBox.Empty();

    const Vector<SplineComponent::SplinePoint*>& controlPoints = splineComponent->GetControlPoints();
    int32 controlPointsCount = static_cast<int32>(controlPoints.size());
    if (controlPointsCount < 2) //not ready spline
        return;
    int32 resSlicesCount = 0;
    splineRenderData->segmentData.clear();
    splineRenderData->segmentData.resize(controlPointsCount);
    //GFX_COMPLETE dte count distance as DWORD num_splits = (DWORD)ceil(m_spline_num_segments * distance(point_1->m_position, point_2->m_position));
    for (int32 i = 0; i < (controlPointsCount - 1); ++i)
    {
        float32 segmentDist = (controlPoints[i + 1]->position.xy() - controlPoints[i]->position.xy()).Length();
        resSlicesCount += int32(ceil(segmentDist / decalComponent->splineSegmentationDistance));
    }
    resSlicesCount++; //closing point

    splineRenderData->sliceCount = resSlicesCount;

    //Allocate arrays
    //GFX_COMPLETE - vertex layout for spline should be driven by material, VTDecalPageRenderer::VTDecalVertex is temp solution
    Vector<VTDecalPageRenderer::VTDecalVertex> vertexData;
    vertexData.reserve(resSlicesCount * 3);

    float32 uTiling = 1.0f / decalComponent->GetSplineTextureDistance();

    int32 curSlice = 0;
    float32 currUCoord = 0.0f;
    for (int32 i = 0; i < (controlPointsCount - 1); ++i)
    {
        const SplineComponent::SplinePoint& sp0 = i == 0 ? *controlPoints[0] : *controlPoints[i - 1];
        const SplineComponent::SplinePoint& sp1 = *controlPoints[i];
        const SplineComponent::SplinePoint& sp2 = *controlPoints[i + 1];
        const SplineComponent::SplinePoint& sp3 = i == (controlPointsCount - 2) ? *controlPoints[controlPointsCount - 1] : *controlPoints[i + 2];

        Vector2 p1 = sp1.position.xy();
        Vector2 p2 = sp2.position.xy();

        //for now tangent solution from BaseSpline - later may be changed to different tangent calculation
        Vector2 r1 = (sp2.position.xy() - sp0.position.xy()) * 0.5f;
        Vector2 r2 = (sp3.position.xy() - sp1.position.xy()) * 0.5f;

        Vector2 a = 2 * p1 - 2 * p2 + r1 + r2;
        Vector2 b = -3 * p1 + 3 * p2 - 2 * r1 - r2;
        Vector2 c = r1;
        Vector2 d = p1;

        float32 sliceLength = (controlPoints[i + 1]->position.xy() - controlPoints[i]->position.xy()).Length();
        int32 sliceSegments = int32(ceil(sliceLength / decalComponent->splineSegmentationDistance));
        if (i == (controlPointsCount - 2))
            sliceSegments += 1; //closing point

        float32 segmentT = decalComponent->splineSegmentationDistance / sliceLength;
        float32 segmentU = uTiling * sliceLength;

        for (int32 segment = 0; segment < sliceSegments; segment++)
        {
            float32 t = Min(segment * segmentT, 1.0f);
            float32 t2 = t * t;
            float32 t3 = t2 * t;

            Vector2 res = a * t3 + b * t2 + c * t + d;
            Vector2 derivative = 3.0f * a * t2 + 2.0f * b * t + c;
            derivative.Normalize();
            Vector2 norm = Vector2(derivative.y, -derivative.x);
            Vector4 tangents = Vector4(derivative.x, derivative.y, norm.x, norm.y);
            float32 width = Lerp(sp1.width, sp2.width, t);
            float32 value = Lerp(sp1.value, sp2.value, t);

            Vector2 pLeft = res - norm * width;
            Vector2 pRight = res + norm * width;
            float32 uCoord = currUCoord + t * segmentU;

            vertexData.push_back({ pLeft, Vector3(uCoord, 0.0f, value), tangents });
            vertexData.push_back({ res, Vector3(uCoord, 0.5f, value), tangents });
            vertexData.push_back({ pRight, Vector3(uCoord, 1.0f, value), tangents });

            splineRenderData->segmentData[i].bbox.AddPoint(res);
            splineRenderData->segmentData[i].bbox.AddPoint(pLeft);
            splineRenderData->segmentData[i].bbox.AddPoint(pRight);
        }
        splineRenderData->resBox.AddAABBox(splineRenderData->segmentData[i].bbox);
        currUCoord += sliceLength * uTiling;
    }

    Vector<uint32> indexData;
    indexData.reserve((resSlicesCount - 1) * 12);
    for (uint32 i = 0, sz = static_cast<uint32>(resSlicesCount - 1); i < sz; i++)
    {
        uint32 base = i * 3;
        indexData.push_back(base + 0);
        indexData.push_back(base + 3);
        indexData.push_back(base + 4);

        indexData.push_back(base + 0);
        indexData.push_back(base + 4);
        indexData.push_back(base + 1);

        indexData.push_back(base + 1);
        indexData.push_back(base + 4);
        indexData.push_back(base + 5);

        indexData.push_back(base + 1);
        indexData.push_back(base + 5);
        indexData.push_back(base + 2);
    }
    rhi::VertexBuffer::Descriptor vbDesc;
    rhi::IndexBuffer::Descriptor ibDesc;

    vbDesc.size = static_cast<uint32>(sizeof(VTDecalPageRenderer::VTDecalVertex) * vertexData.size());
    vbDesc.initialData = vertexData.data();
    vbDesc.usage = rhi::USAGE_STATICDRAW;

    ibDesc.size = static_cast<uint32>(indexData.size() * 4);
    ibDesc.indexSize = rhi::INDEX_SIZE_32BIT;
    ibDesc.initialData = indexData.data();
    ibDesc.usage = rhi::USAGE_STATICDRAW;

    splineRenderData->vertexBuffer = rhi::CreateVertexBuffer(vbDesc);
    DVASSERT(splineRenderData->vertexBuffer);
    splineRenderData->indexBuffer = rhi::CreateIndexBuffer(ibDesc);
    DVASSERT(splineRenderData->indexBuffer);
}

void VTSplineDecalSystem::PrepareForRemove()
{
}
}
