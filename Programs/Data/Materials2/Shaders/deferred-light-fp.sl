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

blending
{
    src = one dst = one
}

fragment_in
{
    float4 p_pos : TEXCOORD0;
    float4 lightPosition : TEXCOORD1; //(pos.xyz, radius)
    float4 lightParams : TEXCOORD2; //(color.rgb, shadowIndex)
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
    fragment_out output;    

#if (ENABLE_DEFERRED_LIGHTS)

    float lightRadius = input.lightPosition.w;

    // compute decal space pos
    float3 ndcPos = input.p_pos.xyz / input.p_pos.w;
    float2 texPos = ndcPos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    texPos = (texPos * viewportSize + viewportOffset) / renderTargetSize;     
    
#if USE_FRAMEBUFFER_FETCH
    float4 g0 = FramebufferFetch(0);
    float4 g1 = FramebufferFetch(1);
    float4 g2 = FramebufferFetch(2);
    float4 g3 = FramebufferFetch(3);
#else
    float4 g0 = tex2D(gBuffer0, texPos);
    float4 g1 = tex2D(gBuffer1, texPos);
    float4 g2 = tex2D(gBuffer2, texPos);
    float4 g3 = tex2D(gBuffer3, texPos);
#endif

    ndcPos.z = (g3.x - ndcToZMapping.y) / ndcToZMapping.x;
    float4 wp = mul(float4(ndcPos, 1.0), invViewProjMatrix);
    float3 worldPos = wp.xyz / wp.w;

    float3 directionToLight = input.lightPosition.xyz - worldPos;
    float distanceToLight = length(directionToLight);

    float3 result;
    if (distanceToLight > lightRadius)
    {
        result = float3(0.0, 0.0, 0.0);
    }
    else
    {
        ResolveInputValues resolve;
        resolve.n = normalize(g1.rgb * 2.0 - 1.0);
        resolve.v = normalize(cameraPosition - worldPos);
        resolve.NdotV = dot(resolve.n, resolve.v);
        resolve.baseColor = g0.rgb;
        resolve.roughness = g2.x;
        resolve.metallness = g2.y;
        resolve.ambientOcclusion = g2.z * g0.w;
        resolve.worldPosition = worldPos;
        resolve.cameraPosition = cameraPosition;

        // TODO : transmittance
        SurfaceValues surface = BuildSurfaceValues(resolve.baseColor, resolve.roughness, resolve.metallness, resolve.ambientOcclusion, resolve.NdotV, 0.0);

        result = ComputePointLight(float4(input.lightPosition), float4(input.lightParams), resolve, surface);
    }

    output.color = float4(result, 1.0);
    
#else //ENABLE_DEFERRED_LIGHTS
    output.color = float4(0.0, 0.0, 0.0, 0.0);
#endif

    return output;
}
