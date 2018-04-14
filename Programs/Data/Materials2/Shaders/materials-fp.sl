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

#if (ALBEDO_TINT_BLEND_MODE != 0)
uniform sampler2D albedoTint;
#endif

fragment_out
{
#if (FOWARD_WITH_COMBINE)
    float4 color : SV_TARGET1;
#else
    float4 color : SV_TARGET0;
#endif
};

fragment_in
{
    float4 varTexCoord : TEXCOORD0;
#if (!WRITE_SHADOW_MAP)
    float4 worldPosition : TEXCOORD1;
    float4 projectedPosition : TEXCOORD2;
    float4 shadowTexCoord : TEXCOORD3;
    float4 varToCamera : TEXCOORD4;
    float3 tangentToFinal0 : NORMAL0;
    float3 tangentToFinal1 : NORMAL1;
    float3 tangentToFinal2 : NORMAL2;
    #if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
    #endif
    #if (VERTEX_COLOR)
    float4 vertexColor : COLOR1;
    #endif
#endif
};

fragment_out fp_main(fragment_in input)
{
    float4 baseColorSample = tex2D(albedo, input.varTexCoord.xy);

#if (ALBEDO_ALPHA_MASK)
    if (baseColorSample.w < albedoAlphaStep)
        discard;
#endif

    fragment_out output;

#if (WRITE_SHADOW_MAP)

    output.color = 0.0;

#else

    ResolveInputValues resolve;

#if (VERTEX_COLOR == VERTEX_COLOR_MULTIPLY)
    baseColorSample.xyz = lerp(baseColorSample.xyz, baseColorSample.xyz * input.vertexColor.xyz, input.vertexColor.a);
#elif (VERTEX_COLOR == VERTEX_COLOR_SOFT_LIGHT)
    baseColorSample.xyz = lerp(baseColorSample.xyz, SoftLightBlend(baseColorSample.xyz, input.vertexColor.xyz), input.vertexColor.a);
#endif

#if (ALBEDO_TINT_BLEND_MODE == 1)
    float4 albedoTintSample = tex2D(albedoTint, input.varTexCoord.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, baseColorSample.xyz * albedoTintSample.xyz, albedoTintSample.w);
#elif (ALBEDO_TINT_BLEND_MODE == 2)
    float4 albedoTintSample = tex2D(albedoTint, input.varTexCoord.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, SoftLightBlend(baseColorSample.xyz, albedoTintSample.xyz), albedoTintSample.w);
#endif

    float bakedAo = max(albedoMinAOValue, baseColorSample.w);
    float4 normalMapSample = tex2D(normalmap, input.varTexCoord.xy);

#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 xy = (normalMapSample.xy * 2.0 - 1.0) * normalScale;
    float3 nts = float3(xy, clamp(sqrt(1.0 - saturate(dot(xy, xy))), 0.0, 1.0));
    resolve.n = normalize(float3(dot(nts, float3(input.tangentToFinal0)), dot(nts, float3(input.tangentToFinal1)), dot(nts, float3(input.tangentToFinal2))));
#else
    resolve.n = normalize(float3(input.tangentToFinal0.z, input.tangentToFinal1.z, input.tangentToFinal2.z));
#endif

#include "include/forward.materials.resolve.h"

#endif

    return output;
}
