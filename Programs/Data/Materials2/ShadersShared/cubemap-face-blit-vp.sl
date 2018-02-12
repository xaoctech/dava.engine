#include "include/common.h"

vertex_in
{
    float3 pos : POSITION;
    float3 uv : TEXCOORD0;
};

vertex_out
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD0;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    output.pos = float4(input.pos.xyz, 1.0);
    output.texCoord = input.uv;
    return output;
}
