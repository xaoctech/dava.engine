#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

#define DECAL_GBUFFER 1
#define DECAL_SNOW 2
#define DECAL_WET 3
#define DECAL_RAIN 4
#define DECAL_GBUFFER_NORMAL 5
#define DECAL_AO 6

#ifndef DECAL_TYPE
#define DECAL_TYPE DECAL_GBUFFER
#endif

#define MODIFY_COLOR ((DECAL_TYPE == DECAL_GBUFFER) || (DECAL_TYPE == DECAL_SNOW) || (DECAL_TYPE == DECAL_WET) || (DECAL_TYPE == DECAL_RAIN))
#define MODIFY_NORMAL ((DECAL_TYPE == DECAL_GBUFFER) || (DECAL_TYPE == DECAL_GBUFFER_NORMAL) || (DECAL_TYPE == DECAL_RAIN))
#define MODIFY_PARAMS ((DECAL_TYPE == DECAL_GBUFFER) || (DECAL_TYPE == DECAL_GBUFFER_NORMAL) || (DECAL_TYPE == DECAL_SNOW) || (DECAL_TYPE == DECAL_WET) || (DECAL_TYPE == DECAL_RAIN) || (DECAL_TYPE == DECAL_AO))

fragment_in
{
    float2 uv : TEXCOORD0;
    float4 p_pos : TEXCOORD1;

    float3 tbnToWorld0 : TEXCOORD2;
    float3 tbnToWorld1 : TEXCOORD3;
    float3 tbnToWorld2 : TEXCOORD4;
    float3 tbnToWorldPos : TEXCOORD5;
    float3 invScale : TEXCOORD6;
};

fragment_out
{
    //GFX_COMPLETE - later mask chanels
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 params : SV_TARGET2;
};

#if DECAL_TYPE == DECAL_RAIN
    #include "include/decals/rain.h" 
#endif

#if (DECAL_TYPE == DECAL_AO) //GFX_COMPLETE later add it to different decals
[material][instance] property float blendWidth = 0.2;
[material][instance] property float aoScale = 0.0; 
#endif

float3 ComputeWorldNormal(float2 normalSample, float3 tbnToWorld0, float3 tbnToWorld1, float3 tbnToWorld2)
{
#if (VIEW_MODE & RESOLVE_NORMAL_MAP)
    float2 xy = (2.0 * normalSample - 1.0) * normalScale;
    float3 nts = float3(xy, sqrt(1.0 - saturate(dot(xy, xy))));
    float3 n = normalize(float3(dot(nts, tbnToWorld0), dot(nts, tbnToWorld1), dot(nts, tbnToWorld2)));    
#else
    float3 n = normalize(float3(tbnToWorld0.z, tbnToWorld1.z, tbnToWorld2.z));
#endif
    return n;
}

fragment_out fp_main(fragment_in input)
{
    //the code above is generic for all decals
    //compute decal space pos
    float3 ndcPos = input.p_pos.xyz / input.p_pos.w;
    float2 texPos = ndcPos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    texPos = (texPos * viewportSize + viewportOffset) / renderTargetSize;
    float g3 = tex2D(gBuffer3, texPos).x;
    ndcPos.z = (g3 - ndcToZMapping.y) / ndcToZMapping.x;

    float4 wp = mul(float4(ndcPos, 1.0), invViewProjMatrix);
    wp.xyz /= wp.w;
    float3 decalPos = mul(wp.xyz - input.tbnToWorldPos, float3x3(input.tbnToWorld0, input.tbnToWorld1, input.tbnToWorld2)) * input.invScale;

    float3 absDecalPos = abs(decalPos);
    if ((absDecalPos.x > 1.0) || (absDecalPos.y > 1.0) || (absDecalPos.z > 1.0))
        discard;

    //compute decal uv's
    float2 uv = decalPos.xy * float2(0.5, -0.5) + float2(0.5, 0.5); //not ndc to uv here as it is in decal space, not ndc space
    uv = uv * uvScale + uvOffset;

    fragment_out output;

//compute specific decals
    
#if DECAL_TYPE == DECAL_GBUFFER
    float4 inColor = tex2D(gBuffer0_copy, texPos);
    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);

    float4 baseColorSample = tex2D(albedo, uv);
    float4 normalMapSample = tex2D(normalmap, uv);

    float3 n = ComputeWorldNormal(normalMapSample.xy, input.tbnToWorld0, input.tbnToWorld1, input.tbnToWorld2);

    float roughness = normalMapSample.z * roughnessScale;
    float metallness = normalMapSample.w * metallnessScale;

    float3 resColor = baseColorSample.xyz * baseColorScale.xyz;
    output.color = float4(lerp(inColor.xyz, resColor, baseColorSample.w), inColor.w);
    output.normal = float4(lerp(inNormal.xyz, n * 0.5 + 0.5, baseColorSample.w), inNormal.w);
    output.params = float4(lerp(inParams.xy, float2(roughness, metallness), baseColorSample.w), inParams.zw);
    
#elif DECAL_TYPE == DECAL_GBUFFER_NORMAL
    float4 inColor = tex2D(gBuffer0_copy, texPos);
    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);

    float4 normalMapSample = tex2D(normalmap, uv);
    float3 n = ComputeWorldNormal(normalMapSample.xy, input.tbnToWorld0, input.tbnToWorld1, input.tbnToWorld2);
    float roughness = normalMapSample.z * roughnessScale;

    output.color = inColor; //float4(lerp(inColor.xyz, n * 0.5 + 0.5, normalMapSample.w), inColor.w);
    output.normal = float4(lerp(inNormal.xyz, n * 0.5 + 0.5, normalMapSample.w), inNormal.w);
    output.params = float4(lerp(inParams.x, roughness, normalMapSample.w), inParams.yzw); 

#elif DECAL_TYPE == DECAL_SNOW

    float4 inColor = tex2D(gBuffer0_copy, texPos);
    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);

    float mask = tex2D(albedo, uv).a; // FP_A8
    float4 normalMapSample = tex2D(normalmap, wp.xy * 2.0);
    float3 microNormal = ComputeWorldNormal(normalMapSample.xy, input.tbnToWorld0, input.tbnToWorld1, input.tbnToWorld2);
    float sparkle_mask = pow(saturate(dot(microNormal, normalize(cameraPosition - wp.xyz))), 20.0);
    float slope_mask = pow(saturate(inNormal.z * 2.0 - 1.0), 0.4);
    float resMask = mask * slope_mask;

    float metallness = lerp(inParams.y, 1.0, sparkle_mask);
    float roughness = lerp(1.0, 0.2, sparkle_mask);
    float reflectance = lerp(0.7, 1.0, sparkle_mask);

    float4 resColor = float4(1.0, 1.0, 1.0, reflectance);
    float4 resParams = float4(roughness, 0.0001 * metallness + inParams.y, inParams.zw);

    output.color = lerp(inColor, resColor, resMask);
    output.normal = inNormal;
    output.params = lerp(inParams, resParams, resMask);

#elif DECAL_TYPE == DECAL_WET

    float4 inColor = tex2D(gBuffer0_copy, texPos);
    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);

    float mask = 2.0 * tex2D(albedo, uv).a; //FP_A8
    mask = saturate((mask - maskThreshold) / maskSoftness);

    output.color = float4(lerp(inColor.rgb, float3(0.0, 0.0, 0.0), mask * 0.4), lerp(inColor.a, 0.2, mask));
    output.normal = inNormal;
    output.params = float4(lerp(inParams.x, 0.1, mask), inParams.yzw);


#elif DECAL_TYPE == DECAL_RAIN

    RainDecalParameters params;
    params.rain_intensity = rain_intensity;
    params.ripples = params_ripples;
    params.flow = params_flow;
    params.time = globalTime;

    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);

    float3 sourceNormal = inNormal.xyz * 2.0 - 1.0;
    float2 resNorm = ComputeRipplesNormal(wp.xyz, uv, sourceNormal, params);
    float3 nts = float3(resNorm, 0.0);
    float3 worldNorm = normalize(float3(dot(nts, input.tbnToWorld0), dot(nts, input.tbnToWorld1), dot(nts, input.tbnToWorld2)) + sourceNormal);

    float4 inColor = tex2D(gBuffer0_copy, texPos + resNorm * 0.01);

    float metalness = inParams.y;
    float darkFactor = 1.0 - saturate((metalness - 0.2) * 1.67);
    float darkScale = saturate(1.4 - inParams.x);

    float3 resColor = inColor.rgb * (1.0 + darkFactor * (darkScale - 1.0));

    //resColor = inNormal.xyz;
    output.color = float4(resColor, inColor.a);
    output.normal = float4(worldNorm * 0.5 + 0.5, inNormal.w);
    output.params = float4(0.1, inParams.yzw);


#elif DECAL_TYPE == DECAL_AO
    float4 inColor = tex2D(gBuffer0_copy, texPos);
    float4 inNormal = tex2D(gBuffer1_copy, texPos);
    float4 inParams = tex2D(gBuffer2_copy, texPos);
    float3 dist_to_edge = float3(1.0, 1.0, 1.0) - absDecalPos;
    float edge_fade_out = min(min(min(dist_to_edge.x, dist_to_edge.y), dist_to_edge.z) / blendWidth, 1.0);

    output.color = inColor;
    output.normal = inNormal;
    output.params = float4(inParams.xy, lerp(inParams.z, inParams.z * aoScale, edge_fade_out), inParams.w);
#endif

    return output;
}
