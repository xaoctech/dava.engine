#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/brdf.h"
#include "include/shadowmapping-v2.h"
#include "include/atmosphere-helpers.h"
#include "include/atmosphere.h"
#include "include/resolve.h"
#include "include/landscape-mask.h"

fragment_in
{
#if LANDSCAPE_USE_INSTANCING
    float3 tangentToFinal0 : TANGENT;
    float3 tangentToFinal1 : BINORMAL;
    float3 tangentToFinal2 : NORMAL;
    float4 texCoord0 : TEXCOORD0;
    float3 texCoord1 : TEXCOORD1;
    float4 shadowTexCoord : TEXCOORD2;
    float3 varToCamera : TEXCOORD3;
    float3 worldPosition : TEXCOORD5;
    float4 projectedPosition : TEXCOORD6;
    #if (LANDSCAPE_LOD_MORPHING && LANDSCAPE_MORPHING_COLOR) || (LANDSCAPE_TESSELLATION_COLOR && LANDSCAPE_MICRO_TESSELLATION)
    float4 vertexColor : COLOR0; // debug patch color
    #endif
    #if LANDSCAPE_PATCHES
    float2 patchTexCoord : COLOR1;
    #endif
#else
    float4 texCoord0 : TEXCOORD0;
#endif
};

fragment_out
{
#if (FOWARD_WITH_COMBINE)
    float4 color : SV_TARGET1;
#else
    float4 color : SV_TARGET0;
#endif
};

#if LANDSCAPE_PICKING_UV
fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.color = float4(input.texCoord1.xy, 0.0, 0.0);
    return output;
}
#else
fragment_out fp_main(fragment_in input)
{
    float4 albedoSample0 = tex2D(albedo, input.texCoord0.xy);
    float4 albedoSample1 = tex2D(albedo, input.texCoord0.zw);
    float4 albedoSample = lerp(albedoSample1, albedoSample0, input.texCoord1.z);
    
#if LANDSCAPE_TOOL
    float4 toolColor = tex2D(toolTexture, input.texCoord1.xy);
#if LANDSCAPE_TOOL_MIX
    albedoSample.rgb = (albedoSample.rgb + toolColor.rgb) / 2.0;
#else
    albedoSample.rgb *= 1.0 - toolColor.a;
    albedoSample.rgb += toolColor.rgb * toolColor.a;
#endif

#endif

#if (LANDSCAPE_LOD_MORPHING && LANDSCAPE_MORPHING_COLOR) || (LANDSCAPE_TESSELLATION_COLOR && LANDSCAPE_MICRO_TESSELLATION)
    albedoSample.xyz = albedoSample.xyz * 0.05 + input.vertexColor.xyz * 0.95;
#endif

#if LANDSCAPE_PATCHES
    #define VT_PAGE_BORDER (1.4 / 150.0)

    float4 edge = float4(VT_PAGE_BORDER, VT_PAGE_BORDER, input.patchTexCoord.x, input.patchTexCoord.y);
    float4 value = float4(input.patchTexCoord.x, input.patchTexCoord.y, 1.0 - VT_PAGE_BORDER, 1.0 - VT_PAGE_BORDER);
    float4 stepresult = step(edge, value);

    float border = 1.0 - stepresult.x * stepresult.y * stepresult.z * stepresult.w;
    albedoSample.rgb = lerp(albedoSample.rgb, float3(0.0, 0.0, 0.0), border);
#endif

#if LANDSCAPE_CURSOR
    float2 cursorCoord = (input.texCoord1.xy - cursorCoordSize.xy) / cursorCoordSize.zw + float2(0.5, 0.5);
    float4 cursorColor = tex2D(cursorTexture, cursorCoord);
    albedoSample.rgb *= 1.0 - cursorColor.a;
    albedoSample.rgb += cursorColor.rgb * cursorColor.a;
#endif

#if LANDSCAPE_COVER_TEXTURE
    float4 coverColor = tex2D(landCoverTexture, input.texCoord1.xy);
    albedoSample.rgb *= 1.0 - coverColor.a;
    albedoSample.rgb += coverColor.rgb * coverColor.a;
#endif

#if LANDSCAPE_CURSOR_V2
    if (landCursorPosition.z > 0.0 && landCursorPosition.w > 0.0)
    {
        float brushFactor = 0.0;
        {
            brushFactor = GetBrushMaskFactor(input.texCoord1.xy, landCursorPosition, cursorRotation, invertFactor);
        }
        albedoSample.rgb *= 1.0 - brushFactor;
        albedoSample.rgb += landCursorColor * brushFactor;
    }
#endif

    float4 saSample = tex2D(shadowaotexture, input.texCoord1.xy);
    float4 normalmapSample0 = tex2D(normalmap, input.texCoord0.xy);
    float4 normalmapSample1 = tex2D(normalmap, input.texCoord0.zw);
    float4 normalmapSample = lerp(normalmapSample1, normalmapSample0, input.texCoord1.z);

    float3 linearColor = SRGBToLinear(albedoSample.rgb);

    ResolveInputValues resolve;
    resolve.vLength = length(input.varToCamera);
    resolve.v = input.varToCamera / resolve.vLength;

#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 nxy = (2.0 * normalmapSample.xy - 1.0);
    float3 nts = float3(nxy, sqrt(1.0 - saturate(dot(nxy, nxy))));
    resolve.n = normalize(float3(dot(nts, input.tangentToFinal0), dot(nts, input.tangentToFinal1), dot(nts, input.tangentToFinal2)));
#else
    resolve.n = normalize(float3(input.tangentToFinal0.z, input.tangentToFinal1.z, input.tangentToFinal2.z));
#endif

    resolve.NdotV = dot(resolve.n, resolve.v);
    resolve.baseColor = linearColor;
    resolve.roughness = normalmapSample.z;
    resolve.metallness = 0.0;
    resolve.ambientOcclusion = saSample.y;
    resolve.directionalLightDirection = lightPosition0.xyz;
    resolve.directionalLightViewSpaceCoords = input.shadowTexCoord;
    resolve.directionalLightColor = lightColor0.xyz / GLOBAL_LUMINANCE_SCALE;
    resolve.directionalLightStaticShadow = SampleStaticShadow(saSample.x, input.texCoord1.xy, lightmapSize) * normalmapSample.w;
    resolve.worldPosition = input.worldPosition;
    resolve.fogParameters = fogParameters.xyz;
    resolve.transmittanceSample = 0.0;
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

    float2 screenSpaceCoords = input.projectedPosition.xy / input.projectedPosition.w * ndcToUvMapping.xy + ndcToUvMapping.zw;
    float4 randomSample = tex2D(noiseTexture64x64, screenSpaceCoords * viewportSize / 64.0) * 2.0 - 1.0;

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

#if (ENABLE_POINT_LIGHTS)
    for (int i = 0; i < pointLightsCount; ++i)
    {
        float4 lightPosition = pointLights[2 * i];
        float4 lightColor = pointLights[2 * i + 1];
        result += ComputePointLight(lightPosition, lightColor, resolve, surface);
    }
#endif

#if (VIEW_MODE & VIEW_TEXTURE_MIP_LEVEL_BIT)
    float2 firstMipSize = tex2Dsize(albedo, 0);
    float mipA = QueryLodLevel(input.texCoord0.xy, firstMipSize);
    float mipB = QueryLodLevel(input.texCoord0.zw, firstMipSize);
    float mip = lerp(mipB, mipA, input.texCoord1.z);
    float3 mipColors[5] = {
        float3(1.0, 1.0, 1.0), /* 0 */
        float3(0.0, 1.0, 0.0), /* 1 */
        float3(0.0, 0.0, 1.0), /* 2 */
        float3(1.0, 0.0, 1.0), /* 3 */
        float3(1.0, 0.0, 0.0) /* 4 */
    };
    float base = floor(mip);
    float next = min(4.0, base + 1.0);
    result = lerp(mipColors[int(base)], mipColors[int(next)], mip - base);
#endif

    fragment_out output;
    output.color = float4(result, 1.0);
    return output;
}
#endif
