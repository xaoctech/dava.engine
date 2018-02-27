#include "include/common.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 inPos : TEXCOORD1;
    float2 uvScale : TEXCOORD2;
};

[auto][global] property float2 viewportSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 renderTargetSize;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 inPos = input.position.xyz;
    float4 inPosition = float4(inPos, 1.0);
    output.position = inPosition;
    output.inPos = inPos.xy;

    float2 texPos = inPos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.uv = (texPos * viewportSize + centerPixelMapping + viewportOffset) / renderTargetSize;
    output.uvScale = viewportSize / renderTargetSize;

    return output;
}
