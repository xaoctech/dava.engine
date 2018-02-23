#pragma once

#include "Base/BaseTypes.h"
#include "Render/Highlevel/QuadRenderer.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
class Light;
class Camera;
class RenderSystem;

/*
 * TODO : move to separate file
 */
class CubemapCameraSetup
{
public:
    bool NextFace();
    void Setup(Camera* camera, float zNear, float zFar);

private:
    uint32 faceIndex = 0;
};

class LightShadowSystem
{
public:
    enum : uint32
    {
        MAX_POINT_LIGHTS = 10000
    };

public:
    LightShadowSystem();
    ~LightShadowSystem();

    void Render(RenderSystem* rs);
    void UpdateLights(RenderSystem* renderSystem, Light* directionalLight, const Vector<Light*> pointLights);

    void InvalidateMaterials();

private:
    void ProcessDirectionalLight(RenderSystem* rs, Light* light);
    void BuildCascades(Light* sourceLight, Camera* viewCamera, const AABBox3& worldBox);
    void CreateFrustumPointsFromCascadeInterval(float intervalBegin, float intervalEnd, const Matrix4& invViewProj, Vector3* worldPoints);

    void ProcessPointLight(RenderSystem* rs, Light* light);
    void ProcessPointLights(RenderSystem* rs, const Vector<Light*> pointLights);

    void DrawBox(Vector3 worldPoints[], RenderSystem* renderSystem, const Color& clr);
    void DrawFrustum(const Matrix4& invViewProj, RenderSystem* renderSystem, const Color& clr);
    void DrawCameraFrustum(Camera* cam, RenderSystem* renderSystem, const Color& clr);
    void GetFrustumCorners(const Matrix4& invViewProj, Vector3* worldPoints);
    void DrawDebug(RenderSystem* renderSystem);

    void ClearPointLightPasses();

private:
    /********************************
     *    Directional light data    *
     ********************************/
    struct ShadowCascadePass;

    using FrustumPoints = Vector3[8];
    struct DirectionalLight
    {
        Matrix4 viewMatrix;
        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
        Vector3 up = Vector3(0.0f, 0.0f, 1.0f);
        Vector2 depthBias = Vector2(0.0f, 0.0f);
        // Only debug stuff goes below
        Matrix4 worldViewProj;
        Array<FrustumPoints, 4> frustums;
        bool drawCascades = false;
        bool drawShadowMap = false;
        bool reverseZEnabled = false;
    } directionalLight;

    ShadowCascadePass* shadowCascadesPass = nullptr;
    Array<Vector4, 4> directionalShadowMapProjectionScale;
    Array<Vector4, 4> directionalShadowMapProjectionOffset;
    Vector4 shadowMapParameters;

private:
    struct PointLightShadowPass;

    struct PointLightInfo
    {
        Vector4 positionRadius = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        Vector4 colorIndex = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    };

    Array<PointLightInfo, MAX_POINT_LIGHTS> pointLights;
    PointLightShadowPass* pointLightPass = nullptr;
    Vector4 lightsCount;
    Vector2 pointLightFaceSize = Vector2(256.0f, 256.0f);
    Vector2 shadowMapViewport = Vector2(0.0f, 0.0f);
    Size2i targetTextureSize = Size2i(0, 0);

    QuadRenderer quadRenderer;
    ScopedPtr<NMaterial> debugMaterial = ScopedPtr<NMaterial>(new NMaterial());
};
}
