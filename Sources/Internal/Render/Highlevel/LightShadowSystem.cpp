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
const float nearClipPlane = -1.0f;
const float farClipPlane = 1.0f;

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
        bool hasValidFrustum = true;
    };
    Vector<Config> configs;
    Vector4* shadowMapParameters = nullptr;
    uint32 cascadesCount = 0;
    uint32 activeCascades = 0;
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
    const Matrix4& viewProjection = viewCamera->GetProjectionMatrix();
    Vector4 proj = Vector4(viewProjection._data[3][0], viewProjection._data[3][1], viewProjection._data[3][2], viewProjection._data[3][3]);

    const int32 cacadesCount = Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES);
    const Size2i smSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
    const float shadowmapSizeWidth = static_cast<float>(smSize.dx);
    Vector4 cascadeIntervals = sourceLight->GetShadowCascadesIntervals();

    bool cascadeSizesValid = ((proj == directionalLight.cameraProjection) && (cascadeIntervals == directionalLight.cascadeIntervals));
    directionalLight.cameraProjection = proj;
    directionalLight.cascadeIntervals = cascadeIntervals;

    shadowCascadesPass->cascadesCount = cacadesCount;
    shadowCascadesPass->shadowMapParameters = &shadowMapParameters;

    directionalLight.position = Vector3::Zero;
    directionalLight.direction = sourceLight->GetDirection();
    directionalLight.up = std::abs(directionalLight.direction.z) < 0.99f ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, -1.0f, 0.0f);
    directionalLight.viewMatrix.BuildLookAtMatrix(directionalLight.position, directionalLight.position + directionalLight.direction, directionalLight.up);

    Vector3 worldCorners[8];
    worldBox.GetCorners(worldCorners);

    Vector3 globalMinExtent(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vector3 globalMaxExtent = -globalMinExtent;
    for (const Vector3& corner : worldCorners)
    {
        Vector3 t = corner * directionalLight.viewMatrix;
        globalMinExtent.x = std::min(globalMinExtent.x, t.x);
        globalMinExtent.y = std::min(globalMinExtent.y, t.y);
        globalMinExtent.z = std::min(globalMinExtent.z, t.z);
        globalMaxExtent.x = std::max(globalMaxExtent.x, t.x);
        globalMaxExtent.y = std::max(globalMaxExtent.y, t.y);
        globalMaxExtent.z = std::max(globalMaxExtent.z, t.z);
    }

    float sceneZNear = -globalMaxExtent.z;
    float sceneZFar = -globalMinExtent.z;
    float projectionFlip = rhi::DeviceCaps().isUpperLeftRTOrigin ? -1.0f : 1.0f;
    float projectionScale = 1.0f / static_cast<float>(cacadesCount);
    float projectionZScale = rhi::DeviceCaps().isZeroBaseClipRange ? 1.0f : 0.5f;
    float projectionZBias = rhi::DeviceCaps().isZeroBaseClipRange ? 0.0f : 0.5f;

    bool revZ = directionalLight.reverseZEnabled;
    bool zeroClip = rhi::DeviceCaps().isZeroBaseClipRange;
    bool invertProjetion = rhi::IsInvertedProjectionRequired(true, false);

    Matrix4 worldProjection = Matrix4::IDENTITY;
    worldProjection.BuildOrtho(globalMaxExtent.x, globalMaxExtent.x, globalMinExtent.y, globalMinExtent.y, sceneZNear, sceneZFar, revZ || zeroClip, revZ);
    directionalLight.worldViewProj = directionalLight.viewMatrix * worldProjection;

    float intervalBegin = 0.0f;
    for (uint32 cascadeIndex = 0; cascadeIndex < shadowCascadesPass->cascadesCount; ++cascadeIndex)
    {
        float intervalEnd = cascadeIntervals.data[cascadeIndex];
        CreateFrustumPointsFromCascadeInterval(intervalBegin, intervalEnd, viewCamera, directionalLight.frustums[cascadeIndex]);

        Vector3 localMinExtent = Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vector3 localMaxExtent = -localMinExtent;
        for (const Vector3& fp : directionalLight.frustums[cascadeIndex])
        {
            Vector3 t = fp * directionalLight.viewMatrix;
            localMinExtent.x = std::min(localMinExtent.x, t.x);
            localMinExtent.y = std::min(localMinExtent.y, t.y);
            localMinExtent.z = std::min(localMinExtent.z, t.z);
            localMaxExtent.x = std::max(localMaxExtent.x, t.x);
            localMaxExtent.y = std::max(localMaxExtent.y, t.y);
            localMaxExtent.z = std::max(localMaxExtent.z, t.z);
        }

        float cascadeZNear = -localMaxExtent.z;
        float cascadeZFar = -localMinExtent.z;
        float zNear = sceneZNear;
        float zFar = std::min(sceneZFar, cascadeZFar);
        shadowCascadesPass->configs[cascadeIndex].hasValidFrustum = (zFar - zNear) >= 0.0f;

        /*
         * Snap to shadow map size to avoid jittering
         * rebuild cascade sizes only when required to avoid floating point precission issues
         */
        if (cascadeSizesValid == false)
        {
            float sz = (directionalLight.frustums[cascadeIndex][0] - directionalLight.frustums[cascadeIndex][6]).Length();
            float tx = sz / shadowmapSizeWidth;
            directionalLight.unitsPerTexel.data[cascadeIndex] = tx;
            directionalLight.cascadeSizes.data[cascadeIndex] = std::floor(sz / tx + 0.5f) * tx;
        }

        localMinExtent.x = std::floor(localMinExtent.x / directionalLight.unitsPerTexel.data[cascadeIndex] + 0.5f) * directionalLight.unitsPerTexel.data[cascadeIndex];
        localMinExtent.y = std::floor(localMinExtent.y / directionalLight.unitsPerTexel.data[cascadeIndex] + 0.5f) * directionalLight.unitsPerTexel.data[cascadeIndex];
        localMaxExtent.x = localMinExtent.x + directionalLight.cascadeSizes.data[cascadeIndex];
        localMaxExtent.y = localMinExtent.y + directionalLight.cascadeSizes.data[cascadeIndex];
        // */

        shadowCascadesPass->configs[cascadeIndex].biasScale = directionalLight.depthBias;

        Camera* camera = shadowCascadesPass->configs[cascadeIndex].camera.Get();
        camera->SetIsOrtho(true);
        camera->SetPosition(directionalLight.position);
        camera->SetDirection(directionalLight.direction);
        camera->SetUp(directionalLight.up);
        camera->SetZNear(zNear);
        camera->SetZFar(zFar);
        camera->Setup(localMinExtent.x, localMaxExtent.x, localMinExtent.y, localMaxExtent.y, zNear, zFar);
        camera->PrepareDynamicParameters(invertProjetion, false);

        Matrix4 projectionMatrix = camera->GetProjectionMatrix();
        projectionMatrix *= Matrix4::MakeScale(Vector3(0.5f, 0.5f * projectionScale * projectionFlip, projectionZScale));
        projectionMatrix *= Matrix4::MakeTranslation(Vector3(0.5f, 0.5f * projectionScale + projectionScale * static_cast<float>(cascadeIndex), projectionZBias));

        directionalShadowMapProjectionScale[cascadeIndex].x = projectionMatrix._data[0][0];
        directionalShadowMapProjectionScale[cascadeIndex].y = projectionMatrix._data[1][1];
        directionalShadowMapProjectionScale[cascadeIndex].z = projectionMatrix._data[2][2];
        directionalShadowMapProjectionScale[cascadeIndex].w = 1.0;
        directionalShadowMapProjectionOffset[cascadeIndex].x = projectionMatrix._data[3][0];
        directionalShadowMapProjectionOffset[cascadeIndex].y = projectionMatrix._data[3][1];
        directionalShadowMapProjectionOffset[cascadeIndex].z = projectionMatrix._data[3][2];
        directionalShadowMapProjectionOffset[cascadeIndex].w = 0.0f;

        intervalBegin = intervalEnd;
    }
}

void LightShadowSystem::CreateFrustumPointsFromCascadeInterval(float intervalBegin, float intervalEnd, Camera* camera, Vector3* worldPoints)
{
    Vector3 origin = camera->GetPosition();
    const Matrix4& m = camera->GetMatrix();

    Vector3 side = Vector3(m._data[0][0], m._data[1][0], m._data[2][0]);
    Vector3 up = Vector3(m._data[0][1], m._data[1][1], m._data[2][1]);
    Vector3 dir = Vector3(-m._data[0][2], -m._data[1][2], -m._data[2][2]);

    float fovRadians = camera->GetFOV() * PI / 180.0f;
    float halfFovTangent = std::tan(fovRadians / 2.0f);

    Vector3 aBegin = side * intervalBegin * halfFovTangent;
    Vector3 bBegin = up * intervalBegin * halfFovTangent / camera->GetAspect();
    Vector3 cBegin = dir * intervalBegin;

    Vector3 aEnd = side * intervalEnd * halfFovTangent;
    Vector3 bEnd = up * intervalEnd * halfFovTangent / camera->GetAspect();
    Vector3 cEnd = dir * intervalEnd;

    worldPoints[0] = origin - aBegin - bBegin + cBegin;
    worldPoints[1] = origin - aBegin + bBegin + cBegin;
    worldPoints[2] = origin + aBegin + bBegin + cBegin;
    worldPoints[3] = origin + aBegin - bBegin + cBegin;
    worldPoints[4] = origin - aEnd - bEnd + cEnd;
    worldPoints[5] = origin - aEnd + bEnd + cEnd;
    worldPoints[6] = origin + aEnd + bEnd + cEnd;
    worldPoints[7] = origin + aEnd - bEnd + cEnd;
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
        const int32 cacadesCount = Renderer::GetRuntimeFlags().GetFlagValue(RuntimeFlags::Flag::SHADOW_CASCADES);

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

        for (int32 i = 0; i < cacadesCount; ++i)
        {
            const ShadowCascadePass::Config& config = shadowCascadesPass->configs[i];
            DrawBox(directionalLight.frustums[i], renderSystem, Color::White);
            DrawCameraFrustum(config.camera.Get(), renderSystem, cascadeColors[i]);
        }
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
        configs[i].viewport = Rect(static_cast<float>(viewportX), static_cast<float>(viewportY), static_cast<float>(viewportW), static_cast<float>(viewportH));
        configs[i].camera->SetValidZInterval(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    }
    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER);
    passConfig.viewport = rhi::Viewport(0, 0, shadowmapSize.dx, shadowmapSize.dy);
    passConfig.depthBias = configs.front().biasScale.x;
    passConfig.depthSlopeScale = configs.front().biasScale.y;
    SetRenderTargetProperties(shadowmapSize.dx, shadowmapSize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_DIRECTIONAL_SHADOW_MAP_DEPTH_BUFFER));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMS, shadowMapParameters->data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}

void LightShadowSystem::ShadowCascadePass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, "ShadowCascadePass");

    Configure();

    if ((cascadesCount > 0) && BeginRenderPass(passConfig))
    {
        for (uint32 i = 0; i < cascadesCount; ++i)
        {
            const Config& config = configs[i];
            if (config.hasValidFrustum == false)
                continue;

            ClearLayersArrays();
            visibilityArray.Clear();

            for (RenderLayer* layer : renderLayers)
            {
                const Rect& vp = config.viewport;
                rhi::Viewport rvp(static_cast<uint32>(vp.x), static_cast<uint32>(vp.y), static_cast<uint32>(vp.dx), static_cast<uint32>(vp.dy));
                layer->SetViewportOverride(true, rvp);
            }

            Camera* camera = config.camera.Get();
            renderSystem->GetRenderHierarchy()->Clip(camera, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA | RenderObject::VISIBLE_SHADOW_CASTER);
            PrepareRenderObjectsToRender(visibilityArray.geometryArray, camera);
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
    viewport.dx = static_cast<float>(shadowmapSize.dx);
    viewport.dy = static_cast<float>(shadowmapSize.dy);
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

            PrepareRenderObjectsToRender(visibilityArray.geometryArray, cam);
            PrepareLayersArrays(visibilityArray.geometryArray, cam);

            Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMS, shadowMapParameters->data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

            SetupCameraParams(cam, cam, nullptr);
            DrawLayers(cam, drawLayersMask);
        }
        EndRenderPass();
    }
}
}
