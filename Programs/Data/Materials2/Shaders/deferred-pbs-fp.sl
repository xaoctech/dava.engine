#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

fragment_in
{
    float4 uv : TEXCOORD0;
    float3 tbnToWorld0 : TEXCOORD1;
    float3 tbnToWorld1 : TEXCOORD2;
    float4 tbnToWorld2 : TEXCOORD3;
    float4 projectedPosition : TEXCOORD4;
#if (VERTEX_BAKED_AO)
    float vertexBakedAO : COLOR0;
#endif
#if (VERTEX_COLOR)
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
    float4 baseColorSample = tex2D(albedo, input.uv.xy);

#if (ALBEDO_ALPHA_MASK)
    if (baseColorSample.w < albedoAlphaStep)
        discard;
#endif

#if (VERTEX_COLOR == VERTEX_COLOR_MULTIPLY)
    baseColorSample.xyz = lerp(baseColorSample.xyz, baseColorSample.xyz * input.vertexColor.xyz, input.vertexColor.a);
#elif (VERTEX_COLOR == VERTEX_COLOR_SOFT_LIGHT)
    baseColorSample.xyz = lerp(baseColorSample.xyz, SoftLightBlend(baseColorSample.xyz, input.vertexColor.xyz), input.vertexColor.a);
#endif

    float4 normalMapSample = tex2D(normalmap, input.uv.xy);

    float ambientOcclusion = max(albedoMinAOValue, baseColorSample.w);

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
    float3 n = normalize(float3(dot(nts, input.tbnToWorld0), dot(nts, input.tbnToWorld1), dot(nts, input.tbnToWorld2.xyz)));
#else
    float3 n = normalize(float3(input.tbnToWorld0.z, input.tbnToWorld1.z, input.tbnToWorld2.z));
#endif

#if (FLIP_BACKFACE_NORMALS)
    n *= sign(input.tbnToWorld2.w);
#endif

    float metallness = normalMapSample.w;
    float roughness = normalMapSample.z * roughnessScale;

#if (TRANSMITTANCE)
    float flags = 1.0;
#else
    float flags = 0.0;
    metallness *= metallnessScale;    
#endif

    fragment_out output;
    output.color = float4(baseColorSample.xyz * baseColorScale.xyz, 1.0 /* UNUSED!!! UNUSED!!! UNUSED!!! */);
    output.normal = float4(n * 0.5 + 0.5, flags);
    output.params = float4(roughness, metallness, saturate((ambientOcclusion + aoBias) * aoScale), directionalLightStaticShadow);
    output.depth = input.projectedPosition.z / input.projectedPosition.w * ndcToZMapping.x + ndcToZMapping.y;
    return output;
}
