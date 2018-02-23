#include "include/common.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    output.varTexCoord0 = input.position.xy;
    output.position = float4(input.position.xy, 1.0, 1.0);
    return output;
}