#include "include/common.h"

#ensuredefined FLIP_Y 0

vertex_in
{
    float3 pos : POSITION;
};

vertex_out
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    output.pos = float4(input.pos.xyz, 1.0);
#if FLIP_Y
    output.uv = input.pos.xy * float2(0.5, -0.5) + 0.5;
#else
    output.uv = input.pos.xy * 0.5 + 0.5;
#endif
    return output;
}
