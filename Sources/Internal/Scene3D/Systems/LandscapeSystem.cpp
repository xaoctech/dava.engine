#include "Scene3D/Systems/LandscapeSystem.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Math/Math2D.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/VirtualTexture.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/LandscapePageManager.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Highlevel/VTDecalManager.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeSystem)
{
    ReflectionRegistrator<LandscapeSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &LandscapeSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 11.0f)]
    .End();
}

LandscapeSystem::LandscapeSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<RenderComponent>())
{
}

LandscapeSystem::~LandscapeSystem()
{
    DVASSERT(landscapeEntities.size() == 0);
}

void LandscapeSystem::AddEntity(Entity* entity)
{
    Landscape* landscapeObject = GetLandscape(entity);
    if (landscapeObject)
    {
        landscapeEntities.push_back(entity);

        const LandscapeQuality* quality = QualitySettingsSystem::Instance()->GetLandscapeQuality(QualitySettingsSystem::Instance()->GetCurLandscapeQuality());
        if (quality)
        {
            LandscapeSubdivision::SubdivisionMetrics metrics;
            metrics.normalMaxHeightError = quality->normalMaxHeightError;
            metrics.normalMaxPatchRadiusError = quality->normalMaxPatchRadiusError;
            metrics.normalMaxAbsoluteHeightError = quality->normalMaxAbsoluteHeightError;

            metrics.zoomMaxHeightError = quality->zoomMaxHeightError;
            metrics.zoomMaxPatchRadiusError = quality->zoomMaxPatchRadiusError;
            metrics.zoomMaxAbsoluteHeightError = quality->zoomMaxAbsoluteHeightError;

            landscapeObject->GetSubdivision()->SetMetrics(metrics);

            if (quality->morphing && landscapeObject->renderMode != Landscape::RENDERMODE_INSTANCING_MORPHING)
            {
                landscapeObject->SetUseMorphing(quality->morphing);
            }
            else
            {
                landscapeObject->GetSubdivision()->UpdateHeightChainData(Rect2i(0, 0, -1, -1));
            }
        }
    }
}

void LandscapeSystem::RemoveEntity(Entity* entity)
{
    uint32 eCount = static_cast<uint32>(landscapeEntities.size());
    for (uint32 e = 0; e < eCount; ++e)
    {
        if (landscapeEntities[e] == entity)
        {
            Landscape* landscapeObject = GetLandscape(entity);
            RemoveExchangingWithLast(landscapeEntities, e);
            break;
        }
    }
}

void LandscapeSystem::PrepareForRemove()
{
    landscapeEntities.clear();
}

void LandscapeSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_LANDSCAPE_SYSTEM);

    Camera* camera = GetScene()->GetRenderSystem()->GetMainCamera();
    for (Entity* e : landscapeEntities)
    {
        Landscape* landscapeObject = GetLandscape(e);
        if (landscapeObject->debugDrawMetrics)
        {
            LandscapeSubdivision* subdivision = landscapeObject->GetSubdivision();
            const LandscapeSubdivision::SubdivisionPatch* subdivisionRoot = subdivision->PrepareSubdivision(camera, landscapeObject->worldTransform);

            DrawPatchMetrics(subdivision, subdivisionRoot);
        }
    }
}

Vector<Landscape*> LandscapeSystem::GetLandscapeObjects()
{
    Vector<Landscape*> landscapes(landscapeEntities.size());
    std::transform(landscapeEntities.begin(), landscapeEntities.end(), landscapes.begin(), [](Entity* e) { return GetLandscape(e); });

    return landscapes;
}

const Vector<Entity*>& LandscapeSystem::GetLandscapeEntities()
{
    return landscapeEntities;
}

void LandscapeSystem::InvalidateVTPages(AABBox3 worldBox)
{
    for (Entity* e : landscapeEntities)
    {
        Landscape* landscape = GetLandscape(e);
        const AABBox3& landscapeBox = landscape->GetWorldBoundingBox();
        if (landscapeBox.IntersectsWithBox(worldBox))
        {
            Vector2 rcpSize = Vector2(1.0f / (landscapeBox.max.x - landscapeBox.min.x), 1.0f / (landscapeBox.max.x - landscapeBox.min.x));
            Vector2 base = (worldBox.min.xy() - landscapeBox.min.xy()) * rcpSize;
            Vector2 size = worldBox.GetSize().xy() * rcpSize;
            landscape->InvalidatePages(Rect(base, size));
        }
    }
}

void LandscapeSystem::DrawPatchMetrics(const LandscapeSubdivision* subdivision, const LandscapeSubdivision::SubdivisionPatch* patch)
{
    if (!patch)
        return;

    if (!patch->isTerminated)
    {
        DrawPatchMetrics(subdivision, patch->children[0]);
        DrawPatchMetrics(subdivision, patch->children[1]);
        DrawPatchMetrics(subdivision, patch->children[2]);
        DrawPatchMetrics(subdivision, patch->children[3]);
    }
    else
    {
        Camera* camera = GetScene()->GetRenderSystem()->GetMainCamera();

        float32 rErrorRel = Min(patch->radiusError / subdivision->maxPatchRadiusError, 1.f);
        float32 hErrorRel = Min(patch->heightError / subdivision->maxHeightError, 1.f);

        RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
        Color color;
        if (rErrorRel > hErrorRel)
        {
            color = Color(0.f, 0.f, 1.f, 1.f);
            drawer->DrawLine(patch->bbox.GetCenter(), patch->bbox.GetCenter() + Vector3(0.f, 0.f, patch->patchRadius), color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        }
        else
        {
            color = Color(1.f, 0.f, 0.f, 1.f);
            drawer->DrawLine(patch->positionOfMaxError - Vector3(0.f, 0.f, patch->maxError), patch->positionOfMaxError, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
            float32 arrowToHeight = Max(patch->positionOfMaxError.z, patch->positionOfMaxError.z - patch->maxError) + patch->patchRadius * .05f;
            Vector3 arrowTo = Vector3(patch->positionOfMaxError.x, patch->positionOfMaxError.y, arrowToHeight);
            drawer->DrawArrow(arrowTo + Vector3(0.f, 0.f, patch->patchRadius * .2f), arrowTo, patch->patchRadius * .05f, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        }

        float32 bboxMiddleH = (patch->bbox.min.z + patch->bbox.max.z) / 2.f;
        Vector3 p0(patch->bbox.min.x, patch->bbox.min.y, bboxMiddleH);
        Vector3 e1(patch->bbox.max.x - patch->bbox.min.x, 0.f, 0.f);
        Vector3 e2(0.f, patch->bbox.max.y - patch->bbox.min.y, 0.f);

        drawer->DrawLine(p0, p0 + e1, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0, p0 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0 + e1, p0 + e1 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(p0 + e2, p0 + e1 + e2, color, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }
}
};
