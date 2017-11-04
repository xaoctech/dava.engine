#include "REPlatform/Scene/Systems/VisibilityCheckSystem.h"

#include "REPlatform/Global/Constants.h"

#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/VisibilityCheckComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>

#include <Math/MathHelpers.h>
#include <Utils/Random.h>

#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/PixelFormatDescriptor.h>
#include <Render/Renderer.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

namespace DAVA
{
namespace VCSInternal
{
static const uint32 CUBEMAPS_POOL_SIZE = 1;
static const uint32 CUBEMAP_SIZE = 2048;
static Array<Texture*, CUBEMAPS_POOL_SIZE> cubemapPool;

Texture* CubemapRenderTargetAtIndex(uint32 index)
{
    DVASSERT(index < CUBEMAPS_POOL_SIZE);
    if (cubemapPool[index] == nullptr)
    {
        const PixelFormatDescriptor& pfd = PixelFormatDescriptor::GetPixelFormatDescriptor(VisibilityCheckRenderer::TEXTURE_FORMAT);
        DVASSERT(rhi::TextureFormatSupported(pfd.format, rhi::PROG_FRAGMENT));
        cubemapPool[index] = Texture::CreateFBO(CUBEMAP_SIZE, CUBEMAP_SIZE, VisibilityCheckRenderer::TEXTURE_FORMAT, true, rhi::TEXTURE_TYPE_CUBE);
        cubemapPool[index]->SetMinMagFilter(rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureFilter::TEXFILTER_LINEAR, rhi::TextureMipFilter::TEXMIPFILTER_NONE);
    }
    return cubemapPool[index];
}
}

VisibilityCheckSystem::VisibilityCheckSystem(Scene* scene)
    : SceneSystem(scene)
    , debugMaterial(new NMaterial())
{
    renderer.SetDelegate(this);
    debugMaterial->SetFXName(FastName("~res:/ResourceEditor/LandscapeEditor/Materials/Distance.Debug2D.material"));
    debugMaterial->PreBuildMaterial(PASS_FORWARD);
}

VisibilityCheckSystem::~VisibilityCheckSystem() = default;

Camera* VisibilityCheckSystem::GetFinalGatherCamera() const
{
    return GetScene()->GetDrawCamera();
}

Camera* VisibilityCheckSystem::GetRenderCamera() const
{
    return GetScene()->GetCurrentCamera();
}

void VisibilityCheckSystem::ReleaseCubemapRenderTargets()
{
    for (auto& cb : VCSInternal::cubemapPool)
    {
        SafeRelease(cb);
    }
}

void VisibilityCheckSystem::Recalculate()
{
    shouldPrerender = true;
    if (renderer.FrameFixed())
    {
        shouldFixFrame = true;
    }
}

void VisibilityCheckSystem::RegisterEntity(Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        AddEntity(entity);
    }
}

void VisibilityCheckSystem::UnregisterEntity(Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        RemoveEntity(entity);
    }
}

void VisibilityCheckSystem::AddEntity(Entity* entity)
{
    auto requiredComponent = entity->GetComponent(Component::VISIBILITY_CHECK_COMPONENT);
    if (requiredComponent != nullptr)
    {
        (static_cast<VisibilityCheckComponent*>(requiredComponent))->Invalidate();
        entitiesWithVisibilityComponent.insert({ entity, Vector<Vector3>() });
        shouldPrerender = true;
        forceRebuildPoints = true;
    }

    auto renderComponent = GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            landscape = static_cast<Landscape*>(ro);
        }
        renderObjectToEntity[ro] = entity;
    }
}

void VisibilityCheckSystem::RemoveEntity(Entity* entity)
{
    auto i = std::find_if(entitiesWithVisibilityComponent.begin(), entitiesWithVisibilityComponent.end(), [entity](const EntityMap::value_type& item) {
        return item.first == entity;
    });

    if (i != entitiesWithVisibilityComponent.end())
    {
        entitiesWithVisibilityComponent.erase(i);
        shouldPrerender = true;
        forceRebuildPoints = true;
    }

    auto renderComponent = GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
        {
            landscape = nullptr;
        }
        renderObjectToEntity.erase(ro);
    }
}

void VisibilityCheckSystem::PrepareForRemove()
{
    entitiesWithVisibilityComponent.clear();
    renderObjectToEntity.clear();
    landscape = nullptr;
    shouldPrerender = true;
    forceRebuildPoints = true;
}

void VisibilityCheckSystem::Process(float32 timeElapsed)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM))
    {
        return;
    }

    bool shouldRebuildIndices = false;

    for (auto& mapItem : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(mapItem.first->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        if (!visibilityComponent->IsValid() || forceRebuildPoints)
        {
            if (visibilityComponent->ShouldRebuildPoints() || forceRebuildPoints)
            {
                BuildPointSetForEntity(mapItem);
                shouldRebuildIndices = true;
            }
            shouldPrerender = true;
            visibilityComponent->SetValid();
        }
    }

    if (shouldRebuildIndices || forceRebuildPoints)
    {
        BuildIndexSet();
    }
    UpdatePointSet();
}

void VisibilityCheckSystem::Draw()
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM))
    {
        return;
    }

    bool enableDebug = false;

    bool shouldRenderOverlay = false;
    RenderSystem* rs = GetScene()->GetRenderSystem();
    RenderHelper* dbg = rs->GetDebugDrawer();
    for (const auto& mapItem : entitiesWithVisibilityComponent)
    {
        VisibilityCheckComponent* visibilityComponent =
        static_cast<VisibilityCheckComponent*>(mapItem.first->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));

        enableDebug |= visibilityComponent->GetDebugDrawEnabled();

        if (visibilityComponent->IsEnabled())
        {
            const Matrix4& worldTransform = mapItem.first->GetWorldTransform();
            Vector3 position = worldTransform.GetTranslationVector();
            Vector3 direction = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            dbg->DrawCircle(position, direction, visibilityComponent->GetRadius(), 36, Color::White, RenderHelper::DRAW_WIRE_DEPTH);
            shouldRenderOverlay = true;
        }
    }

    if (!CacheIsValid())
    {
        BuildCache();
        shouldPrerender = true;
    }

    if (shouldPrerender)
    {
        Prerender();
    }

    for (const auto& point : controlPoints)
    {
        dbg->DrawIcosahedron(point.point, VisibilityCheckRenderer::cameraNearClipPlane,
                             Color(1.0f, 1.0f, 0.5f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        dbg->DrawLine(point.point, point.point + 2.0f * point.normal, Color(0.25f, 1.0f, 0.25f, 1.0f));
    }

    uint32 previousPointIndex = static_cast<uint32>(currentPointIndex);
    for (uint32 cm = 0; (cm < VCSInternal::CUBEMAPS_POOL_SIZE) && (currentPointIndex < controlPoints.size()); ++cm, ++currentPointIndex)
    {
        uint32 pointIndex = controlPointIndices[currentPointIndex];
        const auto& point = controlPoints[pointIndex];
        auto cubemap = VCSInternal::CubemapRenderTargetAtIndex(cm);
        renderer.RenderToCubemapFromPoint(rs, point.point, cubemap);
        renderer.RenderVisibilityToTexture(rs, GetRenderCamera(), GetFinalGatherCamera(), cubemap, point);
    }

    if (shouldFixFrame && (currentPointIndex == controlPoints.size()) && (previousPointIndex < controlPoints.size()))
    {
        renderer.FixFrame();
        shouldFixFrame = false;
    }

    if (shouldRenderOverlay)
    {
        if ((currentPointIndex == controlPoints.size()) || renderer.FrameFixed())
        {
            renderer.RenderCurrentOverlayTexture(rs, GetFinalGatherCamera());
        }

        if (currentPointIndex < controlPoints.size())
        {
            float progress = static_cast<float>(currentPointIndex) / static_cast<float>(controlPoints.size());
            renderer.RenderProgress(progress, shouldFixFrame ? Color(0.25f, 0.5f, 1.0f, 1.0f) : Color::White);
        }
    }

    if (enableDebug)
    {
        Texture* cubemap = VCSInternal::CubemapRenderTargetAtIndex(0);
        if (cubemap)
            RenderSystem2D::Instance()->DrawTexture(cubemap, debugMaterial, Color::White, Rect(2.0f, 2.0f, float32(stateCache.viewportSize.dx) - 4.f, 512.f));

        if (renderer.renderTarget)
            RenderSystem2D::Instance()->DrawTexture(renderer.renderTarget, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, Rect(2.f, 516.f, 256.f, 256.f));

        if (renderer.fixedFrame)
            RenderSystem2D::Instance()->DrawTexture(renderer.fixedFrame, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, Rect(260.f, 516.f, 256.f, 256.f));

        if (renderer.reprojectionTexture)
            RenderSystem2D::Instance()->DrawTexture(renderer.reprojectionTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, Rect(520.f, 516.f, 256.f, 256.f));
    }

    GetFinalGatherCamera()->SetupDynamicParameters(false, nullptr);
}

void VisibilityCheckSystem::InvalidateMaterials()
{
    renderer.InvalidateMaterials();
}

void VisibilityCheckSystem::UpdatePointSet()
{
    controlPoints.clear();

    for (const auto& mapItem : entitiesWithVisibilityComponent)
    {
        auto entity = mapItem.first;
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(entity->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = entity->GetWorldTransform();
            Vector3 position = worldTransform.GetTranslationVector();
            Vector3 normal = MultiplyVectorMat3x3(Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            normal.Normalize();
            float32 upAngle = std::cos(DegToRad(90.0f - visibilityComponent->GetUpAngle()));
            float32 dnAngle = -std::cos(DegToRad(90.0f - visibilityComponent->GetDownAngle()));
            float32 maxDist = visibilityComponent->GetMaximumDistance();
            float32 snapHeight = visibilityComponent->GetHeightAboveLandscape();

            for (const auto& pt : mapItem.second)
            {
                Vector3 placedPoint;
                Vector3 placedNormal;
                Vector3 transformedPoint = position + MultiplyVectorMat3x3(pt, worldTransform);
                if ((landscape != nullptr) && landscape->PlacePoint(transformedPoint, placedPoint, &placedNormal))
                {
                    if (visibilityComponent->ShouldPlaceOnLandscape())
                    {
                        normal = placedNormal;
                        transformedPoint.z = placedPoint.z + snapHeight;
                    }
                    transformedPoint.z = Max(transformedPoint.z, placedPoint.z + 2.0f * VisibilityCheckRenderer::cameraNearClipPlane);
                }

                controlPoints.emplace_back(transformedPoint, normal, GetNormalizedColorForEntity(mapItem), upAngle, dnAngle, maxDist);
            }
        }
    }
}

void VisibilityCheckSystem::Prerender()
{
    renderer.CreateOrUpdateRenderTarget(stateCache.viewportSize);
    renderer.PreRenderScene(GetScene()->GetRenderSystem(), GetFinalGatherCamera());

    currentPointIndex = 0;
    shouldPrerender = false;
}

bool VisibilityCheckSystem::CacheIsValid()
{
    Size2i vpSize(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight());
    if (vpSize != stateCache.viewportSize)
        return false;

    auto cam = GetFinalGatherCamera();
    if (cam != stateCache.camera)
        return false;

    auto currentMatrix = cam->GetViewProjMatrix();
    if (Memcmp(currentMatrix.data, stateCache.viewprojMatrix.data, sizeof(Matrix4)) != 0)
        return false;

    return true;
}

void VisibilityCheckSystem::BuildCache()
{
    stateCache.camera = GetFinalGatherCamera();
    DVASSERT(stateCache.camera != nullptr);

    stateCache.viewportSize = Size2i(Renderer::GetFramebufferWidth(), Renderer::GetFramebufferHeight());
    stateCache.viewprojMatrix = stateCache.camera->GetViewProjMatrix();
}

bool VisibilityCheckSystem::ShouldDrawRenderObject(RenderObject* object)
{
    auto type = object->GetType();

    if (type == RenderObject::TYPE_LANDSCAPE)
        return true;

    if ((type == RenderObject::TYPE_SPEED_TREE) || (type == RenderObject::TYPE_SPRITE) ||
        (type == RenderObject::TYPE_VEGETATION) || (type == RenderObject::TYPE_PARTICLE_EMITTER))
    {
        return false;
    }

    auto entityIterator = renderObjectToEntity.find(object);
    if (entityIterator == renderObjectToEntity.end())
        return false;

    String collisionTypeString = "CollisionType";
    if ((object->GetMaxSwitchIndex() > 0) && (object->GetSwitchIndex() > 0))
    {
        collisionTypeString = "CollisionTypeCrashed";
    }

    VariantType* collisionValue = GetCustomPropertiesValueRecursive(entityIterator->second, collisionTypeString);
    if ((collisionValue == nullptr) || (collisionValue->type != VariantType::TYPE_INT32))
        return false;

    const int32 collisiontype = collisionValue->AsInt32();
    if ((ResourceEditor::ESOT_NO_COLISION == collisiontype) ||
        (ResourceEditor::ESOT_TREE == collisiontype) ||
        (ResourceEditor::ESOT_BUSH == collisiontype) ||
        (ResourceEditor::ESOT_FALLING == collisiontype) ||
        (ResourceEditor::ESOT_FRAGILE_PROJ_INV == collisiontype) ||
        (ResourceEditor::ESOT_SPEED_TREE == collisiontype))
    {
        return false;
    }

    return true;
}

namespace VCSLocal
{
inline bool DistanceFromPointToAnyPointFromSetGreaterThan(const Vector3& pt,
                                                          const Vector<Vector3>& points, float32 distanceSquared, float32 radiusSquared)
{
    float32 distanceFromCenter = pt.x * pt.x + pt.y * pt.y;
    if (distanceFromCenter > radiusSquared)
        return false;

    for (const auto& e : points)
    {
        float32 dx = e.x - pt.x;
        float32 dy = e.y - pt.y;
        if (dx * dx + dy * dy < distanceSquared)
            return false;
    }

    return true;
}

bool TryToGenerateAroundPoint(const Vector3& src, float32 distanceBetweenPoints, float32 radius, Vector<Vector3>& points)
{
    const uint32 maxAttempts = 36;

    float32 distanceSquared = distanceBetweenPoints * distanceBetweenPoints;
    float32 radiusSquared = radius * radius;
    float32 angle = GetEngineContext()->random->RandFloat32InBounds(-PI, PI);
    float32 da = 2.0f * PI / static_cast<float>(maxAttempts);
    uint32 attempts = 0;
    Vector3 newPoint;
    bool canInclude = false;
    do
    {
        newPoint = src + Polar(angle, 2.0f * distanceBetweenPoints);
        if (DistanceFromPointToAnyPointFromSetGreaterThan(newPoint, points, distanceSquared, radiusSquared))
        {
            points.push_back(newPoint);
            return true;
        }
        angle += da;
    } while ((++attempts < maxAttempts) && !canInclude);

    return false;
};
}

void VisibilityCheckSystem::BuildPointSetForEntity(EntityMap::value_type& item)
{
    auto component = static_cast<VisibilityCheckComponent*>(item.first->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));

    float32 radius = component->GetRadius();
    float32 radiusSquared = radius * radius;
    float32 distanceBetweenPoints = component->GetDistanceBetweenPoints();
    float32 distanceSquared = distanceBetweenPoints * distanceBetweenPoints;

    uint32 pointsToGenerate = 2 * static_cast<uint32>(radiusSquared / distanceSquared);
    item.second.clear();
    item.second.reserve(pointsToGenerate);

    if (pointsToGenerate < 2)
    {
        item.second.emplace_back(0.0f, 0.0f, 0.0f);
    }
    else
    {
        item.second.push_back(Polar(GetEngineContext()->random->RandFloat32InBounds(-PI, +PI), radius - distanceBetweenPoints));

        bool canGenerate = true;
        while (canGenerate)
        {
            canGenerate = false;
            for (int32 i = static_cast<int32>(item.second.size()) - 1; i >= 0; --i)
            {
                if (VCSLocal::TryToGenerateAroundPoint(item.second.at(i), distanceBetweenPoints, radius, item.second))
                {
                    canGenerate = true;
                    break;
                }
            }
        }
    }

    float32 verticalVariance = component->GetVerticalVariance();
    if (verticalVariance > 0.0f)
    {
        for (auto& p : item.second)
        {
            p.z = GetEngineContext()->random->RandFloat32InBounds(-verticalVariance, verticalVariance);
        }
    }
}

void VisibilityCheckSystem::BuildIndexSet()
{
    size_t totalPoints = 0;
    for (const auto& item : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<VisibilityCheckComponent*>(item.first->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            totalPoints += item.second.size();
        }
    }
    controlPointIndices.resize(totalPoints);
    for (size_t i = 0; i < totalPoints; ++i)
    {
        controlPointIndices[i] = static_cast<uint32>(i);
    }

    if (totalPoints > 0)
    {
        std::random_shuffle(controlPointIndices.begin() + 1, controlPointIndices.end());
    }

    forceRebuildPoints = false;
}

Color VisibilityCheckSystem::GetNormalizedColorForEntity(const EntityMap::value_type& item) const
{
    auto component = static_cast<VisibilityCheckComponent*>(item.first->GetComponent(Component::VISIBILITY_CHECK_COMPONENT));
    Color normalizedColor = component->GetColor();
    if (component->ShouldNormalizeColor())
    {
        float32 fpoints = static_cast<float>(item.second.size());
        normalizedColor.r = (normalizedColor.r > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.r / fpoints) : 0.0f;
        normalizedColor.g = (normalizedColor.g > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.g / fpoints) : 0.0f;
        normalizedColor.b = (normalizedColor.b > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.b / fpoints) : 0.0f;
    }
    return normalizedColor;
}

void VisibilityCheckSystem::FixCurrentFrame()
{
    if (currentPointIndex == controlPoints.size())
    {
        renderer.FixFrame();
    }
    else
    {
        shouldFixFrame = true;
    }
}

void VisibilityCheckSystem::ReleaseFixedFrame()
{
    renderer.ReleaseFrame();
    shouldFixFrame = false;
}
} // namespace DAVA
