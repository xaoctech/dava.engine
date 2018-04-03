#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/brdf.h"
#include "include/shadowmapping-v2.h"
#include "include/atmosphere.h"
#include "include/resolve.h"
#include "include/landscape-mask.h"

fragment_in
{
#if LANDSCAPE_USE_INSTANCING
    float3 normal : NORMAL0;
    float2 albedoCoord0 : TEXCOORD0;
    float2 albedoCoord1 : TEXCOORD1;
    float2 lightmapTexCoord : TEXCOORD2;
    float4 shadowTexCoord : TEXCOORD2;
    float3 varToCamera : TEXCOORD3;
    float albedoFactor : TEXCOORD4;
#else
    float4 texCoord0 : TEXCOORD0;
#endif
};

fragment_out
{
    float4 color : SV_TARGET0;
};

fragment_out fp_main(fragment_in input)
{
    float4 albedoSample0 = tex2D(albedo, input.albedoCoord0);
    float4 albedoSample1 = tex2D(albedo, input.albedoCoord1);
    float4 baseColorSample = lerp(albedoSample1, albedoSample0, input.albedoFactor);

    fragment_out output;
    {
        float2 bakedShadowAOSample = tex2D(shadowaotexture, input.lightmapTexCoord).xy;
        float bakedShadow = bakedShadowAOSample.x;
        float bakedAo = bakedShadowAOSample.y;
        float3 environmentDiffuseSample = 0.0;
    #if (IB_REFLECTIONS_PREPARE == 0)
        environmentDiffuseSample = sphericalHarmonics[0].xyz * (0.282095);
    #endif
        float LdotN = max(0.0, dot(normalize(input.normal), lightPosition0.xyz));
        float3 result = baseColorSample.xyz * (bakedShadow * LdotN + bakedAo * environmentDiffuseSample);
        output.color = float4(result, 1.0);
    }

    return output;
}
