#include "Render/Highlevel/LightShadowSystem.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Render/DynamicBindings.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Debug/ProfilerGPU.h"

namespace DAVA
{
// TODO : make this depend on API
const float32 nearClipPlane = -1.0f;
const float32 farClipPlane = 1.0f;

/*
 * CubemapCameraSetup
 */
bool CubemapCameraSetup::NextFace()
{
    return (++faceIndex) < 6;
}

void CubemapCameraSetup::Setup(Camera* camera, float zNear, float zFar)
{
    DVASSERT(faceIndex < 6);

    static const DAVA::Vector3 directions[6] =
    {
      DAVA::Vector3(+1.0f, 0.0f, 0.0f),
      DAVA::Vector3(-1.0f, 0.0f, 0.0f),
      DAVA::Vector3(0.0f, +1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, 0.0f, +1.0f),
      DAVA::Vector3(0.0f, 0.0f, -1.0f),
    };
    static const DAVA::Vector3 upVectors[6] =
    {
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, 0.0f, 1.0f),
      DAVA::Vector3(0.0f, 0.0f, -1.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
      DAVA::Vector3(0.0f, -1.0f, 0.0f),
    };

    camera->SetupPerspective(90.0f, 1.0f, zNear, zFar);
    camera->SetDirection(directions[faceIndex]);
    camera->SetUp(upVectors[faceIndex]);
}
/*
 * CubemapCameraSetup end
 */
struct LightShadowSystem::ShadowCascadePass : public RenderPass
{
    ShadowCascadePass(bool enableReverseZ);
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;
    void Configure();

    struct Config
    {
        Rect viewport;
        RefPtr<Camera> camera;
        Vector2 biasScale = Vector2(0.0f, 0.0f);
    };
    Vector<Config> configs;
    Vector4* shadowMapParameters = nullptr;
    uint32 cascadesCount = 0;
};

struct LightShadowSystem::PointLightShadowPass : public RenderPass
{
    PointLightShadowPass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

    struct Config
    {
        Rect viewport;
        RefPtr<Camera> camera;
        Vector2 bias = Vector2(0.0f, 0.0f);
        bool enabled = true;
    };

    Vector<Config> configs;
    Vector4* shadowMapParameters = nullptr;
    bool drawFrustums = false;
    bool drawShadowMap = false;
};

/*********************************************************************************************
 *
 * ShadowSystem implementation
 *
 *********************************************************************************************/
const FastName FLAG_DEBUG_SHOW_DISTANCE = FastName("SHOW_DISTANCE");

LightShadowSystem::LightShadowSystem()
{
    debugMaterial->SetFXName(FastName("~res:/Materials2/ShadowMapDebug.material"));
    debugMaterial->AddFlag(FLAG_DEBUG_SHOW_DISTANCE, 0);

    directionalLight.reverseZEnabled = false;
}

LightShadowSystem::~LightShadowSystem()
{
    ClearPointLightPasses();
    SafeDelete(pointLightPass);
    SafeDelete(shadowCascadesPass);
}

void LightShadowSystem::Render(RenderSystem* rs)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_VIEW, directionalLight.viewMatrix.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PROJECTION_SCALE, directionalShadowMapProjectionScale.data(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PROJECTION_OFFSET, directionalShadowMapProjectionOffset.data(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_LIGHTING_PARAMETERS, lightsCount.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_POINT_LIGHTS, pointLights.data(), DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_POINT_SHADOW_MAP_FACE_SIZE, pointLightFaceSize.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMS, shadowMapParameters.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    shadowCascadesPass->Draw(rs);

    if (pointLightPass != nullptr)
        pointLightPass->Draw(rs);

    DrawDebug(rs);
}

void LightShadowSystem::InvalidateMaterials()
{
    debugMaterial->InvalidateRenderVariants();
}

void LightShadowSystem::UpdateLights(RenderSystem* renderSystem, Light* directionalLight, const Vector<Light*> pointLights)
{
    ProcessDirectionalLight(renderSystem, directionalLight);
    // ProcessPointLights(renderSystem, pointLights);
}

void LightShadowSystem::ProcessPointLight(RenderSystem* renderSystem, Light* light)
{
    pointLightPass->drawFrustums |= light->GetDebugDrawFrustumsEnabled();
    pointLightPass->drawShadowMap |= light->GetDebugDrawShadowMapEnabled();
    CubemapCameraSetup cameraSetup;
    do
    {
        if (light->GetCastsShadow())
        {
            pointLightPass->configs.emplace_back();
            PointLightShadowPass::Config& config = pointLightPass->configs.back();
            config.viewport = Rect(shadowMapViewport.x, shadowMapViewport.y, pointLightFaceSize.x, pointLightFaceSize.y);
            config.camera = RefPtr<Camera>(new Camera);
            config.camera->SetPosition(light->GetPosition());
            config.bias = light->GetShadowWriteBias();
            cameraSetup.Setup(config.camera.Get(), 0.01f, light->GetRadius());
        }

        shadowMapViewport.dx += pointLightFaceSize.x;
        if (shadowMapViewport.dx >= targetTextureSize.dx)
        {
            shadowMapViewport.dx = 0;
            shadowMapViewport.dy += pointLightFaceSize.y;
        }
    } while (cameraSetup.NextFace());
}

void LightShadowSystem::ClearPointLightPasses()
{
    if (pointLightPass != nullptr)
    {
        pointLightPass->drawFrustums = false;
        pointLightPass->drawShadowMap = false;
        pointLightPass->configs.clear();
    }
}

void LightShadowSystem::ProcessPointLights(RenderSystem* renderSystem, const Vector<Light*> pl)
{
    /*
     * point lights are completely disabled now
     *
    if (pointLightPass == nullptr)
        pointLightPass = new PointLightShadowPass();

    ClearPointLightPasses();
    pointLightPass->shadowMapParameters = &shadowMapParameters;

    shadowMapParameters.w = static_cast<float>(Renderer::GetRuntimeTextures().GetDynamicTextureSize(RuntimeTextures::TEXTURE_POINT_LIGHT_LOOKUP).dx);

    targetTextureSize = Renderer::GetRuntimeTextures().GetDynamicTextureSize(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP);
    shadowMapViewport = Vector2(0.0f, 0.0);
    pointLights.fill(PointLightInfo());

    uint32 totalSlots = static_cast<uint32>((targetTextureSize.dx / pointLightFaceSize.x) * (targetTextureSize.dy / pointLightFaceSize.y));
    ClearPointLightPasses();

    uint32 index = 0;
    for (Light* light : pl)
    {
        const Color& clr = light->GetColor();
        const Vector3& pos = light->GetPosition();

        pointLights[index].positionRadius = Vector4(pos, light->GetRadius());
        pointLights[index].colorIndex = Vector4(clr.r, clr.g, clr.b, light->GetCastsShadow() ? static_cast<float>(index) : -1.0f);

        ProcessPointLight(renderSystem, light);
        if (index++ > 9)
            break;
    }
    lightsCount.y = static_cast<float>(index);
    */
}

/********************************************************************************************
 *
 * Directional light processing
 *
 ********************************************************************************************/
void LightShadowSystem::ProcessDirectionalLight(RenderSystem* renderSystem, Light* light)
{
    if (shadowCascadesPass == nullptr)
        shadowCascadesPass = new ShadowCascadePass(directionalLight.reverseZEnabled);

    if ((light != nullptr) && (renderSystem->GetMainCamera() != nullptr) && (light->GetLightType() == Light::eType::TYPE_SUN))
    {
        const Size2i smSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);

        lightsCount.x = 1.0f;
        lightsCount.z = 1.0f;
        directionalLight.direction = light->GetDirection();
        directionalLight.drawCascades = light->GetDebugDrawFrustumsEnabled();
        directionalLight.drawShadowMap = light->GetDebugDrawShadowMapEnabled();
        shadowMapParameters.x = light->GetShadowFilterRadius().x;
        shadowMapParameters.y = light->GetShadowFilterRadius().y;
        shadowMapParameters.z = static_cast<float>(smSize.dx);
        shadowMapParameters.w = static_cast<float>(smSize.dy);
        directionalLight.depthBias = light->GetShadowWriteBias();
        BuildCascades(light, renderSystem->GetMainCamera(), renderSystem->GetRenderHierarchy()->GetWorldBoundingBox());
    }
    else
    {
        lightsCount.x = 0.0f;
        lightsCount.z = 0.0f;
        directionalLight.drawCascades = false;
        directionalLight.drawShadowMap = false;
        shadowMapParameters.x = 0.0f;
        shadowMapParameters.y = 0.0f;
        shadowMapParameters.z = 0.0f;
        shadowMapParameters.w = 0.0f;
    }
}

void LightShadowSystem::BuildCascades(Light* sourceLight, Camera* viewCamera, const AABBox3& worldBox)
{
    const int32 cacadesCount = Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES);
    const Size2i smSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
    const float shadowmapSizeWidth = static_cast<float>(smSize.dx);
    Vector4 cascadeIntervals = sourceLight->GetShadowCascadesIntervals();

    shadowCascadesPass->shadowMapParameters = &shadowMapParameters;
    shadowCascadesPass->cascadesCount = cacadesCount;

    Matrix4 invViewProj;
    viewCamera->PrepareDynamicParameters(rhi::IsInvertedProjectionRequired(true, false), directionalLight.reverseZEnabled);
    viewCamera->GetViewProjMatrix(false, directionalLight.reverseZEnabled).GetInverse(invViewProj);

    directionalLight.direction = sourceLight->GetDirection();
    directionalLight.up = std::abs(directionalLight.direction.z) < 0.99f ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, -1.0f, 0.0f);
    directionalLight.viewMatrix.BuildLookAtMatrix(Vector3(0.0f, 0.0f, 0.0f), directionalLight.direction, directionalLight.up);

    Vector3 worldCorners[8];
    worldBox.GetCorners(worldCorners);

    Vector3 minExtent(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector3 maxExtent = -minExtent;
    for (const Vector3& corner : worldCorners)
    {
        Vector3 t = corner * directionalLight.viewMatrix;
        minExtent.x = std::min(minExtent.x, t.x);
        minExtent.y = std::min(minExtent.y, t.y);
        minExtent.z = std::min(minExtent.z, t.z);
        maxExtent.x = std::max(maxExtent.x, t.x);
        maxExtent.y = std::max(maxExtent.y, t.y);
        maxExtent.z = std::max(maxExtent.z, t.z);
    }

    float zNear = -maxExtent.z;
    float zFar = -minExtent.z;

    bool revZ = directionalLight.reverseZEnabled;
    bool zeroClip = rhi::DeviceCaps().isZeroBaseClipRange;
    Matrix4 worldProjection = Matrix4::IDENTITY;
    worldProjection.BuildOrtho(minExtent.x, maxExtent.x, minExtent.y, maxExtent.y, zNear, zFar, revZ || zeroClip, revZ);
    directionalLight.worldViewProj = directionalLight.viewMatrix * worldProjection;

    float intervalBegin = 0.0f;
    for (uint32 cascadeIndex = 0; cascadeIndex < shadowCascadesPass->cascadesCount; ++cascadeIndex)
    {
        CreateFrustumPointsFromCascadeInterval(intervalBegin, cascadeIntervals.data[cascadeIndex], invViewProj, directionalLight.frustums[cascadeIndex]);

        Vector3 minExtent = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vector3 maxExtent = -minExtent;
        for (const Vector3& fp : directionalLight.frustums[cascadeIndex])
        {
            Vector3 t = fp * directionalLight.viewMatrix;
            minExtent.x = std::min(minExtent.x, t.x);
            minExtent.y = std::min(minExtent.y, t.y);
            minExtent.z = std::min(minExtent.z, t.z);
            maxExtent.x = std::max(maxExtent.x, t.x);
            maxExtent.y = std::max(maxExtent.y, t.y);
            maxExtent.z = std::max(maxExtent.z, t.z);
        }

        /*
         * Snap to shadow map size to avoid jittering
         */
        float32 diagonal = (directionalLight.frustums[cascadeIndex][0] - directionalLight.frustums[cascadeIndex][6]).Length();
        Vector3 borderOffset = 0.5f * (Vector3(diagonal, diagonal, diagonal) - (maxExtent - minExtent));
        maxExtent += borderOffset;
        minExtent -= borderOffset;
        float32 unitsPerTexel = diagonal / shadowmapSizeWidth;
        minExtent.x = std::floor(minExtent.x / unitsPerTexel) * unitsPerTexel;
        minExtent.y = std::floor(minExtent.y / unitsPerTexel) * unitsPerTexel;
        maxExtent.x = std::floor(maxExtent.x / unitsPerTexel) * unitsPerTexel;
        maxExtent.y = std::floor(maxExtent.y / unitsPerTexel) * unitsPerTexel;
        // */

        shadowCascadesPass->configs[cascadeIndex].biasScale = directionalLight.depthBias;
        shadowCascadesPass->configs[cascadeIndex].camera->SetPosition(directionalLight.position);
        shadowCascadesPass->configs[cascadeIndex].camera->SetDirection(directionalLight.direction);
        shadowCascadesPass->configs[cascadeIndex].camera->SetUp(directionalLight.up);
        shadowCascadesPass->configs[cascadeIndex].camera->Setup(minExtent.x, maxExtent.x, minExtent.y, maxExtent.y, zNear, zFar);
        shadowCascadesPass->configs[cascadeIndex].camera->PrepareDynamicParameters(rhi::IsInvertedProjectionRequired(true, false), false);

        float flip = rhi::DeviceCaps().isUpperLeftRTOrigin ? -1.0f : 1.0f;
        float scale = 1.0f / static_cast<float>(cacadesCount);
        float zScale = rhi::DeviceCaps().isZeroBaseClipRange ? 1.0f : 0.5f;
        float zBias = rhi::DeviceCaps().isZeroBaseClipRange ? 0.0f : 0.5f;

        Matrix4 projectionMatrix = shadowCascadesPass->configs[cascadeIndex].camera->GetProjectionMatrix();
        projectionMatrix *= Matrix4::MakeScale(Vector3(0.5f, 0.5f * scale * flip, zScale));
        projectionMatrix *= Matrix4::MakeTranslation(Vector3(0.5f, 0.5f * scale + scale * static_cast<float>(cascadeIndex), zBias));

        directionalShadowMapProjectionScale[cascadeIndex].x = projectionMatrix._data[0][0];
        directionalShadowMapProjectionScale[cascadeIndex].y = projectionMatrix._data[1][1];
        directionalShadowMapProjectionScale[cascadeIndex].z = projectionMatrix._data[2][2];
        directionalShadowMapProjectionScale[cascadeIndex].w = 1.0;
        directionalShadowMapProjectionOffset[cascadeIndex].x = projectionMatrix._data[3][0];
        directionalShadowMapProjectionOffset[cascadeIndex].y = projectionMatrix._data[3][1];
        directionalShadowMapProjectionOffset[cascadeIndex].z = projectionMatrix._data[3][2];
        directionalShadowMapProjectionOffset[cascadeIndex].w = 0.0f;

        if (cascadeIndex > 0)
        {
            intervalBegin = cascadeIntervals.data[cascadeIndex - 1];
        }
    }
}

void LightShadowSystem::CreateFrustumPointsFromCascadeInterval(float intervalBegin, float intervalEnd, const Matrix4& invViewProj, Vector3* worldPoints)
{
    Vector4 nearPlaneCenter = Vector4(0.0f, 0.0f, nearClipPlane, 1.0f) * invViewProj;
    nearPlaneCenter /= nearPlaneCenter.w;

    Vector4 farPlaneCenter = Vector4(0.0f, 0.0f, farClipPlane, 1.0f) * invViewProj;
    farPlaneCenter /= farPlaneCenter.w;

    Vector3 origin = nearPlaneCenter.GetVector3();
    Vector3 direction = (farPlaneCenter - nearPlaneCenter).GetVector3();
    direction.Normalize();

    Plane nearPlane(direction, origin + direction * intervalBegin);
    Plane farPlane(direction, origin + direction * intervalEnd);

    float nearDistance = 0.0f;
    float farDistance = 0.0f;
    {
        Vector4 nearLeftBottom = Vector4(-1.0f, -1.0f, nearClipPlane, 1.0f) * invViewProj;
        Vector4 farLeftBottom = Vector4(-1.0f, -1.0f, farClipPlane, 1.0f) * invViewProj;
        farLeftBottom /= farLeftBottom.w;
        nearLeftBottom /= nearLeftBottom.w;
        Vector3 lb = (farLeftBottom - nearLeftBottom).GetVector3();
        lb.Normalize();

        nearDistance = 0.0f;
        farDistance = 0.0f;
        nearPlane.IntersectByRay(nearLeftBottom.GetVector3(), lb, nearDistance);
        farPlane.IntersectByRay(nearLeftBottom.GetVector3(), lb, farDistance);
        worldPoints[0] = nearLeftBottom.GetVector3() + nearDistance * lb;
        worldPoints[4] = nearLeftBottom.GetVector3() + farDistance * lb;
    }
    {
        Vector4 nearLeftTop = Vector4(-1.0f, 1.0f, nearClipPlane, 1.0f) * invViewProj;
        Vector4 farLeftTop = Vector4(-1.0f, 1.0f, farClipPlane, 1.0f) * invViewProj;
        farLeftTop /= farLeftTop.w;
        nearLeftTop /= nearLeftTop.w;
        Vector3 lt = (farLeftTop - nearLeftTop).GetVector3();
        lt.Normalize();

        nearDistance = 0.0f;
        farDistance = 0.0f;
        nearPlane.IntersectByRay(nearLeftTop.GetVector3(), lt, nearDistance);
        farPlane.IntersectByRay(nearLeftTop.GetVector3(), lt, farDistance);
        worldPoints[1] = (nearLeftTop.GetVector3() + nearDistance * lt);
        worldPoints[5] = (nearLeftTop.GetVector3() + farDistance * lt);
    }
    {
        Vector4 nearRightTop = Vector4(1.0f, 1.0f, nearClipPlane, 1.0f) * invViewProj;
        Vector4 farRightTop = Vector4(1.0f, 1.0f, farClipPlane, 1.0f) * invViewProj;
        farRightTop /= farRightTop.w;
        nearRightTop /= nearRightTop.w;
        Vector3 rt = (farRightTop - nearRightTop).GetVector3();
        rt.Normalize();

        nearDistance = 0.0f;
        farDistance = 0.0f;
        nearPlane.IntersectByRay(nearRightTop.GetVector3(), rt, nearDistance);
        farPlane.IntersectByRay(nearRightTop.GetVector3(), rt, farDistance);
        worldPoints[2] = (nearRightTop.GetVector3() + nearDistance * rt);
        worldPoints[6] = (nearRightTop.GetVector3() + farDistance * rt);
    }
    {
        Vector4 nearRightBottom = Vector4(1.0f, -1.0f, nearClipPlane, 1.0f) * invViewProj;
        Vector4 farRightBottom = Vector4(1.0f, -1.0f, farClipPlane, 1.0f) * invViewProj;
        farRightBottom /= farRightBottom.w;
        nearRightBottom /= nearRightBottom.w;
        Vector3 rb = (farRightBottom / farRightBottom.w - nearRightBottom / nearRightBottom.w).GetVector3();
        rb.Normalize();

        nearDistance = 0.0f;
        farDistance = 0.0f;
        nearPlane.IntersectByRay(nearRightBottom.GetVector3(), rb, nearDistance);
        farPlane.IntersectByRay(nearRightBottom.GetVector3(), rb, farDistance);
        worldPoints[3] = (nearRightBottom.GetVector3() + nearDistance * rb);
        worldPoints[7] = (nearRightBottom.GetVector3() + farDistance * rb);
    }
}

/*********************************************************************************************
 *
 * Debug methods implementation
 *
 *********************************************************************************************/
void LightShadowSystem::DrawFrustum(const Matrix4& invViewProj, RenderSystem* renderSystem, const Color& clr)
{
    Vector3 worldPoints[8];
    GetFrustumCorners(invViewProj, worldPoints);
    DrawBox(worldPoints, renderSystem, clr);
}

void LightShadowSystem::DrawCameraFrustum(Camera* cam, RenderSystem* renderSystem, const Color& clr)
{
    Matrix4 invViewProj;
    cam->GetViewProjMatrix(false, cam->GetReverseZEnabled()).GetInverse(invViewProj);
    DrawFrustum(invViewProj, renderSystem, clr);
}

void LightShadowSystem::DrawBox(Vector3 worldPoints[], RenderSystem* renderSystem, const Color& clr)
{
    // near plane
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[0], worldPoints[1], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[1], worldPoints[2], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[2], worldPoints[3], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[3], worldPoints[0], clr);

    // far plane
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[4], worldPoints[5], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[5], worldPoints[6], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[6], worldPoints[7], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[7], worldPoints[4], clr);

    // left edges
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[0], worldPoints[4], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[1], worldPoints[5], clr);

    // right edges
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[2], worldPoints[6], clr);
    renderSystem->GetDebugDrawer()->DrawLine(worldPoints[3], worldPoints[7], clr);
}

void LightShadowSystem::GetFrustumCorners(const Matrix4& invViewProj, Vector3* worldPoints)
{
    Vector4 nearLeftBottom = Vector4(-1.0f, -1.0f, nearClipPlane, 1.0f) * invViewProj;
    worldPoints[0] = (nearLeftBottom / nearLeftBottom.w).GetVector3();

    Vector4 nearLeftTop = Vector4(-1.0f, 1.0f, nearClipPlane, 1.0f) * invViewProj;
    worldPoints[1] = (nearLeftTop / nearLeftTop.w).GetVector3();

    Vector4 nearRightTop = Vector4(1.0f, 1.0f, nearClipPlane, 1.0f) * invViewProj;
    worldPoints[2] = (nearRightTop / nearRightTop.w).GetVector3();

    Vector4 nearRightBottom = Vector4(1.0f, -1.0f, nearClipPlane, 1.0f) * invViewProj;
    worldPoints[3] = (nearRightBottom / nearRightBottom.w).GetVector3();

    Vector4 farLeftBottom = Vector4(-1.0f, -1.0f, farClipPlane, 1.0f) * invViewProj;
    worldPoints[4] = (farLeftBottom / farLeftBottom.w).GetVector3();

    Vector4 farLeftTop = Vector4(-1.0f, 1.0f, farClipPlane, 1.0f) * invViewProj;
    worldPoints[5] = (farLeftTop / farLeftTop.w).GetVector3();

    Vector4 farRightTop = Vector4(1.0f, 1.0f, farClipPlane, 1.0f) * invViewProj;
    worldPoints[6] = (farRightTop / farRightTop.w).GetVector3();

    Vector4 farRightBottom = Vector4(1.0f, -1.0f, farClipPlane, 1.0f) * invViewProj;
    worldPoints[7] = (farRightBottom / farRightBottom.w).GetVector3();
}

void LightShadowSystem::DrawDebug(RenderSystem* renderSystem)
{
    if (directionalLight.drawCascades)
    {
        static const Color cascadeColors[] =
        {
          Color(1.0f, 0.0f, 0.0f, 1.0f),
          Color(0.0f, 1.0f, 0.0f, 1.0f),
          Color(0.0f, 0.0f, 1.0f, 1.0f),
          Color(1.0f, 0.0f, 1.0f, 1.0f),
        };

        Matrix4 invViewProj;
        directionalLight.worldViewProj.GetInverse(invViewProj);
        DrawFrustum(invViewProj, renderSystem, Color(1.0f, 1.0f, 0.0f, 1.0f));

        for (FrustumPoints& fp : directionalLight.frustums)
            DrawBox(fp, renderSystem, Color::White);

        uint32 index = 0;
        for (const ShadowCascadePass::Config& config : shadowCascadesPass->configs)
            DrawCameraFrustum(config.camera.Get(), renderSystem, cascadeColors[index++]);
    }

    if (directionalLight.drawShadowMap)
    {
        Size2i shadowmapSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);

        float gap = 10.0f;
        float dstWidth = static_cast<float>(Renderer::GetFramebufferWidth());
        float dstHeight = static_cast<float>(Renderer::GetFramebufferHeight());
        float dstSize = std::floor(std::min(dstWidth, dstHeight) - 2.0f * gap);

        QuadRenderer::Options options;
        options.material = debugMaterial;
        options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        options.srcTexture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
        options.srcTexSize = Vector2(static_cast<float>(shadowmapSize.dx), static_cast<float>(shadowmapSize.dy));
        options.srcRect = Rect2f(0.0f, 0.0f, static_cast<float>(shadowmapSize.dx), static_cast<float>(shadowmapSize.dy));
        options.dstTexture = rhi::HTexture();
        options.dstTexSize = Vector2(dstWidth, dstHeight);

        float scaleX = std::min(1.0f, options.srcTexSize.dx / options.srcTexSize.dy);
        float scaleY = std::min(1.0f, options.srcTexSize.dy / options.srcTexSize.dx);
        options.dstRect = Rect2f(gap, gap, dstSize * scaleX, dstSize * scaleY);

        options.renderPassPriority = -150;
        options.material->SetFlag(FLAG_DEBUG_SHOW_DISTANCE, 0);
        quadRenderer.Render(options);
    }

    static const Color faceColors[6] =
    {
      Color(1.0f, 0.0f, 0.0f, 1.0f),
      Color(0.5f, 0.0f, 0.0f, 1.0f),
      Color(0.0f, 1.0f, 0.0f, 1.0f),
      Color(0.0f, 0.5f, 0.0f, 1.0f),
      Color(0.0f, 0.0f, 1.0f, 1.0f),
      Color(0.0f, 0.0f, 0.5f, 1.0f),
    };

    uint32 faceIndex = 0;
    if ((pointLightPass != nullptr) && pointLightPass->drawFrustums)
    {
        for (PointLightShadowPass::Config& config : pointLightPass->configs)
        {
            if (config.enabled)
            {
                DrawCameraFrustum(config.camera.Get(), renderSystem, faceColors[(faceIndex++) % 6]);
            }
        }
    }

    /*
     * Point shadow maps are completely disabled now
     *
    if ((pointLightPass != nullptr) && pointLightPass->drawShadowMap)
    {
        Size2i shadowmapSize = Renderer::GetRuntimeTextures().GetDynamicTextureSize(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP);

        float gap = 10.0f;
        float dstWidth = static_cast<float>(Renderer::GetFramebufferWidth());
        float dstHeight = static_cast<float>(Renderer::GetFramebufferHeight());
        float dstSize = std::floor(std::min(dstHeight, dstWidth) - 2.0f * gap);

        QuadRenderer::Options options;
        options.material = debugMaterial;
        options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        options.srcTexture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP);
        options.srcTexSize = Vector2(static_cast<float>(shadowmapSize.dx), static_cast<float>(shadowmapSize.dy));
        options.srcRect = Rect2f(0.0f, 0.0f, static_cast<float>(shadowmapSize.dx), static_cast<float>(shadowmapSize.dy));
        options.dstTexture = rhi::HTexture();
        options.dstTexSize = Vector2(dstWidth, dstHeight);
        options.dstRect = Rect2f(gap, gap, dstSize, dstSize);
        options.renderPassPriority = -150;
        options.material->SetFlag(FLAG_DEBUG_SHOW_DISTANCE, 1);
        quadRenderer.Render(options);
    }
    */
}

/*********************************************************************************************
 *
 * ShadowCascadePass implementation
 *
 *********************************************************************************************/
LightShadowSystem::ShadowCascadePass::ShadowCascadePass(bool enableReverseZ)
    : RenderPass(FastName("ShadowCascadePass"))
{
    passConfig.priority = PRIORITY_SERVICE_3D + 10;
    // GFX_COMPLETE - 10 is base priority that is set for ui3dview in RE - later us priority set from outside

    passConfig.colorBuffer[0].texture = rhi::InvalidHandle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;
    passConfig.colorBuffer[0].clearColor[0] = 1.0f;
    passConfig.colorBuffer[0].clearColor[1] = 1.0f;
    passConfig.colorBuffer[0].clearColor[2] = 1.0f;
    passConfig.colorBuffer[0].clearColor[3] = 1.0f;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    passConfig.depthBias = 0.0f;
    passConfig.depthSlopeScale = 0.0f;
    passConfig.usesReverseDepth = enableReverseZ;

    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));

    configs.resize(4);
    for (auto& cfg : configs)
    {
        cfg.camera.ConstructInplace();
        cfg.camera->SetIsOrtho(true);
    }
}

void LightShadowSystem::ShadowCascadePass::Configure()
{
    Size2i shadowmapSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
    int32 cacadesCount = Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES);
    for (int32 i = 0; i < cacadesCount; ++i)
    {
        float viewportX = 0.0f;
        float viewportW = static_cast<float>(shadowmapSize.dx);
        float viewportH = static_cast<float>(shadowmapSize.dy / cacadesCount);
        float viewportY = viewportH * static_cast<float>(i);
        configs[i].viewport = Rect(static_cast<float>(viewportX), static_cast<float>(viewportY), static_cast<float32>(viewportW), static_cast<float32>(viewportH));
        configs[i].camera->SetValidZInterval(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    }
    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
    passConfig.viewport = rhi::Viewport(0, 0, shadowmapSize.dx, shadowmapSize.dy);
    passConfig.depthBias = configs.front().biasScale.x;
    passConfig.depthSlopeScale = configs.front().biasScale.y;
    SetRenderTargetProperties(shadowmapSize.dx, shadowmapSize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER));
}

void LightShadowSystem::ShadowCascadePass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, "ShadowCascadePass");

    Configure();

    if (BeginRenderPass(passConfig))
    {
        for (uint32 i = 0; i < cascadesCount; ++i)
        {
            Camera* camera = configs[i].camera.Get();
            const Rect& vp = configs[i].viewport;
            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMS, shadowMapParameters->data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

            for (RenderLayer* layer : renderLayers)
            {
                rhi::Viewport rvp(static_cast<uint32>(vp.x), static_cast<uint32>(vp.y), static_cast<uint32>(vp.dx), static_cast<uint32>(vp.dy));
                layer->SetViewportOverride(true, rvp);
            }

            ClearLayersArrays();
            visibilityArray.Clear();
            renderSystem->GetRenderHierarchy()->Clip(camera, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_SHADOW_CASTER);
            PrepareLayersArrays(visibilityArray.geometryArray, camera);

            SetupCameraParams(camera, camera, nullptr);
            DrawLayers(camera, drawLayersMask);
        }

        EndRenderPass();
    }
}

/*
 * PointLightShadowPass implementation
 */
LightShadowSystem::PointLightShadowPass::PointLightShadowPass()
    : RenderPass(FastName("PointLightShadowPass"))
{
    /*
     * Point light shadows are completely disabled now
     *
    Size2i shadowmapSize = Renderer::GetRuntimeTextures().GetDynamicTextureSize(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP);

    passConfig.priority = PRIORITY_SERVICE_3D + 10;
    passConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP);
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = std::numeric_limits<float>::max();
    passConfig.colorBuffer[0].clearColor[1] = std::numeric_limits<float>::max();
    passConfig.colorBuffer[0].clearColor[2] = std::numeric_limits<float>::max();
    passConfig.colorBuffer[0].clearColor[3] = std::numeric_limits<float>::max();
    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetDynamicTexture(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP_DEPTH_BUFFER);
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    viewport.x = 0;
    viewport.y = 0;
    viewport.dx = static_cast<float32>(shadowmapSize.dx);
    viewport.dy = static_cast<float32>(shadowmapSize.dy);
    SetViewport(viewport);

    SetRenderTargetProperties(shadowmapSize.dx, shadowmapSize.dy, Renderer::GetRuntimeTextures().GetDynamicTextureFormat(RuntimeTextures::TEXTURE_POINT_SHADOW_MAP));

    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
    */
}

void LightShadowSystem::PointLightShadowPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, "PointLightShadowPass");

    if ((configs.size() > 0) && BeginRenderPass(passConfig))
    {
        for (PointLightShadowPass::Config& config : configs)
        {
            const Rect& vp = config.viewport;
            Camera* cam = config.camera.Get();

            for (RenderLayer* layer : renderLayers)
            {
                rhi::Viewport rvp(static_cast<uint32>(vp.x), static_cast<uint32>(vp.y), static_cast<uint32>(vp.dx), static_cast<uint32>(vp.dy));
                layer->SetViewportOverride(true, rvp);
            }

            ClearLayersArrays();
            visibilityArray.Clear();
            renderSystem->GetRenderHierarchy()->Clip(cam, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_SHADOW_CASTER);
            PrepareLayersArrays(visibilityArray.geometryArray, cam);

            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMS, shadowMapParameters->data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

            SetupCameraParams(cam, cam, nullptr);
            DrawLayers(cam, drawLayersMask);
        }
        EndRenderPass();
    }
}
}
