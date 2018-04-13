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

fragment_out
{
    float4 color : SV_TARGET0;
};

fragment_in
{
    float4 texCoords : TEXCOORD0;
    float4 projectedPosition : TEXCOORD1;
#if (WRITE_SHADOW_MAP == 0)
    float3 varToCamera : TEXCOORD2;
    float4 shadowTexCoord : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
#endif
    float frameDiff : TEXCOORD5;
};

fragment_out fp_main(fragment_in input)
{
    float2 screenSpaceCoords = (input.projectedPosition.xy / input.projectedPosition.w * 0.5 + 0.5) * viewportSize;
    float jitteredFrameDiff = GetDitherPatternValue4x4(1.0 - input.frameDiff, screenSpaceCoords);
    
    float4 s0 = tex2D(albedo, input.texCoords.xy);
    float4 s1 = tex2D(albedo, input.texCoords.zw);
    float4 baseColorSample = lerp(s0, s1, jitteredFrameDiff);
    
    float alphaTreshold = 2.0 / 255.0;
    if (baseColorSample.w < alphaTreshold) discard;
    
    fragment_out output;
    
#if (WRITE_SHADOW_MAP)
    
    output.color = float4(0.0, 0.0, 0.0, 1.0);
    
#else
    
    float4 n0 = tex2D(normalmap, input.texCoords.xy);
    float4 n1 = tex2D(normalmap, input.texCoords.zw);
    float4 normalMapSample = lerp(n0, n1, jitteredFrameDiff);
    
    float2 nXY = normalMapSample.xy * 2.0 - 1.0;
    float nZ = sqrt(1.0 - saturate(dot(nXY, nXY)));
    
    ResolveInputValues resolve;
    resolve.n = normalize(input.tangent * nXY.x + input.bitangent * nXY.y + input.normal * nZ);
    
    float bakedAo = baseColorSample.w;
    
#define USE_BAKED_LIGHTING 0
#define VERTEX_BAKED_AO 0
#define FLIP_BACKFACE_NORMALS 0
#define TRANSMITTANCE 1
    
#include "include/forward.materials.resolve.h"
    
#endif
    
    return output;
}
