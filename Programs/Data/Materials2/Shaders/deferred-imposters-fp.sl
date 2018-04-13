#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;
    float4 texCoords : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 bitangent : TEXCOORD4;
    float frameDiff : TEXCOORD5;
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
    float2 screenSpaceCoords = (input.projectedPosition.xy / input.projectedPosition.w * 0.5 + 0.5) * viewportSize;
    float4 randomSample = tex2D(noiseTexture64x64, screenSpaceCoords / 64.0);
    float t = input.frameDiff * input.frameDiff;
    float jitteredFrameDiff = step(randomSample.z, t);
    // jitteredFrameDiff = GetDitherPatternValue4x4(1.0 - t, screenSpaceCoords);
    // jitteredFrameDiff = GetDitherPatternValue8x8(1.0 - t, screenSpaceCoords);

    float4 s0 = tex2D(albedo, input.texCoords.xy);
    float4 s1 = tex2D(albedo, input.texCoords.zw);
    float4 albedoSample = lerp(s0, s1, jitteredFrameDiff);
    
    float alphaTreshold = 2.0 / 255.0;
    if (albedoSample.w < alphaTreshold) discard;

    float4 n0 = tex2D(normalmap, input.texCoords.xy);
    float4 n1 = tex2D(normalmap, input.texCoords.zw);
    float4 normalSample = lerp(n0, n1, jitteredFrameDiff);
    
    normalSample.xy = normalSample.xy * 2.0 - 1.0;
    float nZ = sqrt(1.0 - saturate(dot(normalSample.xy, normalSample.xy)));
    
    float3 normal = normalize(input.tangent * normalSample.x + input.bitangent * normalSample.y + input.normal * nZ);

    float roughness = normalSample.z;
    float transmittance = normalSample.w;
    float ao = albedoSample.w;
    float staticShadow = 1.0; // no static shadow
    float flags = 1.0; // transmittance enabled by default

    fragment_out output;
    output.color = float4(albedoSample.xyz, 1.0);
    output.normal = float4(normal * 0.5 + 0.5, flags);
    output.params = float4(roughness, transmittance, ao, staticShadow);
    output.depth = input.projectedPosition.z / input.projectedPosition.w * ndcToZMapping.x + ndcToZMapping.y;
    return output;
}
