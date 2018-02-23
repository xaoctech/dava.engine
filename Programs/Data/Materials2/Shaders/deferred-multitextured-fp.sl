#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#if (VERTEX_BLEND_TEXTURES)
#include "include/vertex-color-tex-blend.h"
#else
#include "include/atlas.h"
#endif

#if (ALBEDO_MODIFIER_BLEND_MODE != 0)
uniform sampler2D albedoModifier;
#endif

fragment_in
{
    float4 uv : TEXCOORD0;
    float3 tbnToWorld0 : TEXCOORD1;
    float3 tbnToWorld1 : TEXCOORD2;
    float3 tbnToWorld2 : TEXCOORD3;
    float4 projectedPosition : TEXCOORD4;
#if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
#endif
#if (VERTEX_COLOR || VERTEX_BLEND_TEXTURES)
    float4 vertexColor : COLOR1;
#endif
};

fragment_out
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 params : SV_TARGET2;
    float4 depth : SV_TARGET3;
};

fragment_out fp_main(fragment_in input)
{
#if (!VERTEX_BLEND_TEXTURES)
    AtlasSampleResult atlasSampleResult = SampleAtlas(input.uv);
    float4 baseColorSample = atlasSampleResult.color;
    float4 normalMapSample = tex2Dgrad(normalmap, atlasSampleResult.sampleUv, atlasSampleResult.uvddx, atlasSampleResult.uvddy);
#else
    TexturesBlendResult blendRes = BlendTexturesVertexColor(input.uv.xy, input.vertexColor);
    float4 baseColorSample = blendRes.albedo;
    float4 normalMapSample = blendRes.normal;
#endif

#if (ALBEDO_ALPHA_MASK)
    if (baseColorSample.w < albedoAlphaStep)
        discard;
#endif

#if (VERTEX_COLOR)
    baseColorSample.xyz *= lerp(float3(1.0, 1.0, 1.0), input.vertexColor.xyz, input.vertexColor.a);
    baseColorSample.xyz = saturate(baseColorSample.xyz * 2.0);
#endif

#if (ALBEDO_MODIFIER_BLEND_MODE == 1)
    float4 albedoModifierSample = tex2D(albedoModifier, input.uv.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, baseColorSample.xyz * albedoModifierSample.xyz, albedoModifierSample.w);
#elif (ALBEDO_MODIFIER_BLEND_MODE == 2)
    float4 albedoModifierSample = tex2D(albedoModifier, input.uv.zw);
    baseColorSample.xyz = lerp(baseColorSample.xyz, SoftLightBlend(baseColorSample.xyz, albedoModifierSample.xyz), albedoModifierSample.w);
#endif

    float ambientOcclusion = albedoMinAOValue;

#if (VERTEX_BAKED_AO)
    ambientOcclusion *= input.vertexBakedAO;
#endif

#if (USE_BAKED_LIGHTING)
    float4 prebakedShadowAOSample = tex2D(shadowaotexture, input.uv.zw);
    float directionalLightStaticShadow = ApplyCanvasCheckers(prebakedShadowAOSample.x, input.uv.zw / shadowaoUV.zw, shadowaoSize);
    ambientOcclusion *= prebakedShadowAOSample.y;
#else
    float directionalLightStaticShadow = 1.0;
#endif
   
#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 xy = (2.0 * normalMapSample.xy - 1.0) * normalScale;
    float3 nts = float3(xy, sqrt(1.0 - saturate(dot(xy, xy))));
    float3 n = normalize(float3(dot(nts, input.tbnToWorld0), dot(nts, input.tbnToWorld1), dot(nts, input.tbnToWorld2)));
#else
    float3 n = normalize(float3(input.tbnToWorld0.z, input.tbnToWorld1.z, input.tbnToWorld2.z));
#endif

    float metallness = normalMapSample.w;
    float roughness = normalMapSample.z * roughnessScale;

    fragment_out output;

#if TRANSMITTANCE
    float flags = 1.0;
#else
    float flags = 0.0;
    metallness *= metallnessScale;    
#endif

    output.color = float4(baseColorSample.xyz * baseColorScale.xyz, 1.0);
    output.normal = float4(n * 0.5 + 0.5, flags);
    output.params = float4(roughness, metallness, saturate((ambientOcclusion + aoBias) * aoScale), directionalLightStaticShadow);
    output.depth = input.projectedPosition.z / input.projectedPosition.w * ndcToZMapping.x + ndcToZMapping.y;
    return output;
}
