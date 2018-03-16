#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/brdf.h"
#include "include/shadowmapping-v2.h"
#include "include/atmosphere.h"
#include "include/resolve.h"

#ensuredefined USE_FRAMEBUFFER_FETCH 0

fragment_in
{
    float2 uv : TEXCOORD0;
    float2 p_pos : TEXCOORD1;
};

fragment_out
{    
#if USE_FRAMEBUFFER_FETCH
    float4 color : SV_TARGET4;
#else
    float4 color : SV_TARGET0;
#endif
};

fragment_out fp_main(fragment_in input)
{
#if USE_FRAMEBUFFER_FETCH
    float4 g0 = FramebufferFetch(0);
    float4 g1 = FramebufferFetch(1);
    float4 g2 = FramebufferFetch(2);
    float4 g3 = FramebufferFetch(3); 
#else
    float4 g0 = tex2D(gBuffer0, input.uv);
    float4 g1 = tex2D(gBuffer1, input.uv);
    float4 g2 = tex2D(gBuffer2, input.uv);
    float4 g3 = tex2D(gBuffer3, input.uv);
#endif

    float isTransmittanceMaterial = step(0.5, g1.a);

    float3 ndcPos = float3(input.p_pos, (g3.x - ndcToZMapping.y) / ndcToZMapping.x);
    float4 worldPos = mul(float4(ndcPos, 1.0), invViewProjMatrix);
    worldPos /= worldPos.w;

    ResolveInputValues resolve;
    resolve.worldPosition = worldPos.xyz;
    resolve.n = normalize(g1.rgb * 2.0 - 1.0);
    resolve.v = cameraPosition - worldPos.xyz;
    resolve.vLength = length(resolve.v);
    resolve.v /= resolve.vLength;
    resolve.NdotV = dot(resolve.n, resolve.v);
    resolve.baseColor = SRGBToLinear(g0.xyz);
    resolve.roughness = g2.x;
    resolve.metallness = g2.y;
    resolve.ambientOcclusion = g2.z;
    resolve.directionalLightDirection = lightPosition0.xyz;
    resolve.directionalLightViewSpaceCoords = mul(worldPos, shadowView);
    resolve.directionalLightColor = lightColor0.xyz / globalLuminanceScale;
    resolve.directionalLightStaticShadow = g2.w;
    resolve.transmittanceSample = resolve.metallness * isTransmittanceMaterial;
    resolve.metallness *= 1.0 - isTransmittanceMaterial;
    resolve.fogParameters = fogParameters.xyz;
    resolve.cameraPosition = cameraPosition;

#if (PARALLAX_CORRECTED)
    resolve.localProbeCaptureWorldToLocalMatrix = localProbeCaptureWorldToLocalMatrix;
    resolve.localProbeCapturePositionInWorldSpace = localProbeCapturePositionInWorldSpace;
#endif

    SurfaceValues surface = BuildSurfaceValues(resolve.baseColor, resolve.roughness, resolve.metallness, resolve.ambientOcclusion, resolve.NdotV, resolve.transmittanceSample);

    resolve.environmentDiffuseSample = 0.0;
#if (!IB_REFLECTIONS_PREPARE)
    #if (USE_DOMINANT_DIRECTION)
    float3 diffuseSampleDirection = GetDiffuseDominantDirection(resolve.n, resolve.v, resolve.NdotV, surface.roughness);
    #else
    float3 diffuseSampleDirection = resolve.n;
    #endif
    resolve.environmentDiffuseSample += sphericalHarmonics[0].xyz * (0.282095);
    resolve.environmentDiffuseSample += sphericalHarmonics[1].xyz * (0.488603 * diffuseSampleDirection.y);
    resolve.environmentDiffuseSample += sphericalHarmonics[2].xyz * (0.488603 * diffuseSampleDirection.z);
    resolve.environmentDiffuseSample += sphericalHarmonics[3].xyz * (0.488603 * diffuseSampleDirection.x);
    resolve.environmentDiffuseSample += sphericalHarmonics[4].xyz * (1.092548 * diffuseSampleDirection.x * diffuseSampleDirection.y);
    resolve.environmentDiffuseSample += sphericalHarmonics[5].xyz * (1.092548 * diffuseSampleDirection.y * diffuseSampleDirection.z);
    resolve.environmentDiffuseSample += sphericalHarmonics[6].xyz * (0.315392 * (3.0 * diffuseSampleDirection.z * diffuseSampleDirection.z - 1.0));
    resolve.environmentDiffuseSample += sphericalHarmonics[7].xyz * (1.092548 * diffuseSampleDirection.x * diffuseSampleDirection.z);
    resolve.environmentDiffuseSample += sphericalHarmonics[8].xyz * (0.546274 * (diffuseSampleDirection.x * diffuseSampleDirection.x - diffuseSampleDirection.y * diffuseSampleDirection.y));
#endif

    float4 randomSample = tex2D(noiseTexture64x64, (ndcPos.xy * 0.5 + 0.5) * viewportSize / 64.0) * 2.0 - 1.0;

    ShadowParameters shadow;
    for (int i = 0; i < SHADOW_CASCADES; ++i)
    {
        shadow.cascadesProjectionScale[i] = directionalShadowMapProjectionScale[i].xyz;
        shadow.cascadesProjectionOffset[i] = directionalShadowMapProjectionOffset[i].xyz;
    }
    shadow.rotationKernel = randomSample.xy;
    shadow.filterRadius = shadowMapParameters.xy;
    shadow.shadowMapSize = shadowMapParameters.zw;

    float3 result = ResolveFinalColor(resolve, surface, shadow);
    
    fragment_out output;
    output.color = float4(result, 1.0);
    return output;
}
