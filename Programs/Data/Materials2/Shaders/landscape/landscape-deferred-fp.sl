#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/brdf.h"
#include "include/shadowmapping-v2.h"
#include "include/landscape-mask.h"

uniform sampler2D albedo;
uniform sampler2D normalmap;
uniform sampler2D shadowaotexture;
uniform sampler2D noiseTexture64x64;
uniform samplerCUBE globalReflection;
uniform samplerCUBE localReflection;

fragment_in
{
    float3 tangentToFinal0 : TANGENT;
    float3 tangentToFinal1 : BINORMAL;
    float3 tangentToFinal2 : NORMAL;
    float4 texCoord0 : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float pageBlend : TEXCOORD2;
    float4 projectedPosition : TEXCOORD3;
    #if (LANDSCAPE_LOD_MORPHING && LANDSCAPE_MORPHING_COLOR) || (LANDSCAPE_TESSELLATION_COLOR && LANDSCAPE_MICRO_TESSELLATION)
    float4 vertexColor : COLOR0; // debug patch color
    #endif
    #if LANDSCAPE_PATCHES
    float2 patchTexCoord : COLOR1;
    #endif
};

fragment_out
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 params : SV_TARGET2;
    float4 depth : SV_TARGET3;
};

#if LANDSCAPE_TOOL
uniform sampler2D toolTexture;
#endif

#if LANDSCAPE_CURSOR
uniform sampler2D cursorTexture;
[material][instance] property float4 cursorCoordSize = float4(0, 0, 1, 1);
#endif

fragment_out fp_main(fragment_in input)
{
    float4 albedoSample0 = tex2D(albedo, input.texCoord0.xy);
    float4 albedoSample1 = tex2D(albedo, input.texCoord0.zw);
    float4 albedoSample = lerp(albedoSample1, albedoSample0, input.pageBlend);
    float4 normalmapSample0 = tex2D(normalmap, input.texCoord0.xy);
    float4 normalmapSample1 = tex2D(normalmap, input.texCoord0.zw);
    float4 normalmapSample = lerp(normalmapSample1, normalmapSample0, input.pageBlend);
    float4 saSample = tex2D(shadowaotexture, input.texCoord1.xy);

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

#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 nxy = (2.0 * normalmapSample.xy - 1.0);
    float3 nts = float3(nxy, sqrt(1.0 - saturate(dot(nxy, nxy))));
    float3 n = normalize(float3(dot(nts, input.tangentToFinal0), dot(nts, input.tangentToFinal1), dot(nts, input.tangentToFinal2)));
#else
    float3 n = normalize(float3(input.tangentToFinal0.z, input.tangentToFinal1.z, input.tangentToFinal2.z));
#endif

    float roughness = normalmapSample.z;

    float directionalLightStaticShadow = SampleStaticShadow(saSample.x, input.texCoord1.xy, lightmapSize);
    float ambientOcclusion = saSample.y;

    fragment_out output;
    output.color = albedoSample;
    output.normal = float4(n * 0.5 + 0.5, 0.0);
    output.params = float4(roughness, 0.0, ambientOcclusion, directionalLightStaticShadow * normalmapSample.w);
    output.depth = input.projectedPosition.z / input.projectedPosition.w * ndcToZMapping.x + ndcToZMapping.y;
    return output;
}
