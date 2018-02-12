#include "include/common.h"

vertex_in
{
    [vertex] float3 position : POSITION;
    [instance] float4 lightPosition : TEXCOORD0; //(pos.xyz, radius)
    [instance] float4 lightParams : TEXCOORD1; //(color.rgb, shadowIndex)
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 p_pos : TEXCOORD0;
    float4 lightPosition : TEXCOORD1; //(pos.xyz, radius)
    float4 lightParams : TEXCOORD2; //(color.rgb, shadowIndex)
};

[auto][instance] property float4x4 viewProjMatrix;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float lightRadius = input.lightPosition.w;
    float3 in_pos = input.position.xyz * lightRadius + input.lightPosition.xyz;
    float4 inPosition = float4(in_pos, 1.0);
    float4 resPosition = mul(inPosition, viewProjMatrix);
    output.position = resPosition;
    output.p_pos = resPosition;
    output.lightPosition = input.lightPosition;
    output.lightParams = input.lightParams;

    return output;
}
