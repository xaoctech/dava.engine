#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/brdf.h"
#include "include/shadowmapping-v2.h"
#include "include/atmosphere.h"
#include "include/resolve.h"

#if (VERTEX_BLEND_TEXTURES)
#include "include/vertex-color-tex-blend.h"
#else
#include "include/atlas.h"
#endif

#if (ALBEDO_MODIFIER_BLEND_MODE != 0)
uniform sampler2D albedoModifier;
#endif

fragment_out
{
    float4 color : SV_TARGET0;
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
    #if (VERTEX_COLOR || VERTEX_BLEND_TEXTURES)
    float4 vertexColor : COLOR1;
    #endif
#endif
};

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

#if (WRITE_SHADOW_MAP)

    output.color = 0.0;

#else

    ResolveInputValues resolve;

#if (!VERTEX_BLEND_TEXTURES)
    AtlasSampleResult atlasSampleResult = SampleAtlas(input.varTexCoord);
    float4 baseColorSample = atlasSampleResult.color;
    float4 normalMapSample = tex2Dgrad(normalmap, atlasSampleResult.sampleUv, atlasSampleResult.uvddx, atlasSampleResult.uvddy);
#else
    TexturesBlendResult blendRes = BlendTexturesVertexColor(input.varTexCoord.xy, input.vertexColor);
    float4 baseColorSample = blendRes.albedo;
    float4 normalMapSample = blendRes.normal;
#endif

#if (VERTEX_COLOR)
    baseColorSample.xyz *= lerp(float3(1.0, 1.0, 1.0), input.vertexColor.xyz, input.vertexColor.a);
    baseColorSample.xyz = saturate(baseColorSample.xyz * 2.0);
#endif

#if (ALBEDO_MODIFIER_BLEND_MODE == 1)
    float4 albedoModifierSample = tex2D(albedoModifier, input.varTexCoord.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, baseColorSample.xyz * albedoModifierSample.xyz, albedoModifierSample.w);
#elif (ALBEDO_MODIFIER_BLEND_MODE == 2)
    float4 albedoModifierSample = tex2D(albedoModifier, input.varTexCoord.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, SoftLightBlend(baseColorSample.xyz, albedoModifierSample.xyz), albedoModifierSample.w);
#endif

    float bakedAo = albedoMinAOValue;

#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 xy = (normalMapSample.xy * 2.0 - 1.0) * normalScale;
    float3 nts = float3(xy, clamp(sqrt(1.0 - saturate(dot(xy, xy))), 0.0, 1.0));
    resolve.n = normalize(float3(dot(nts, input.tangentToFinal0), dot(nts, input.tangentToFinal1), dot(nts, input.tangentToFinal2)));
#else
    resolve.n = normalize(float3(input.tangentToFinal0.z, input.tangentToFinal1.z, input.tangentToFinal2.z));
#endif

#define LDR_FLOW 1
#include "include/forward.materials.resolve.h"

#endif

    return output;
}
