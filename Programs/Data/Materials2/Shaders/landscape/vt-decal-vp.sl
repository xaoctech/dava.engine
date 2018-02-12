#include "include/common.h"

vertex_in
{
    [vertex] float2 position : POSITION;
    [vertex] float3 uv : TEXCOORD0;
    [vertex] float4 tangents : TEXCOORD2; //local space (t, b)
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 uv : TEXCOORD0;
    float4 basis : TEXCOORD1; //(t, b);
    float value : TEXCOORD2;
};

[auto][instance] property float4 vtPageInfo; //(pos.xy, size.xy)
[auto][instance] property float4 vtBasis; //2d rotation
[auto][instance] property float2 vtPos; //2d position

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float2 inPosition = vtBasis.xy * input.position.x + vtBasis.zw * input.position.y + vtPos;
    float2 pagePosition = (inPosition - vtPageInfo.xy) / vtPageInfo.zw;

    output.position = float4(pagePosition * 2.0 - float2(1.0, 1.0), 0.0, 1.0);
    output.uv.xy = input.uv;
    output.uv.zw = pagePosition;
    output.value = input.uv.z;
    output.basis = float4(input.tangents.x * vtBasis.xy + input.tangents.y * vtBasis.zw, input.tangents.z * vtBasis.xy + input.tangents.w * vtBasis.zw);

    return output;
}
