#include "include/common.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 p_pos : TEXCOORD1;
};

[auto][global] property float2 viewportSize;
[auto][global] property float2 viewportOffset;
[auto][global] property float2 renderTargetSize;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float3 in_pos = input.position.xyz;
    float4 inPosition = float4(in_pos, 1.0);
    output.position = inPosition;
    output.p_pos = in_pos.xy;

    float2 texPos = in_pos.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
    output.uv = (texPos * viewportSize + centerPixelMapping + viewportOffset) / renderTargetSize;

    return output;
}
