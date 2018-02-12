#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

vertex_in
{
    float3 pos : POSITION;
};

vertex_out
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;
    output.pos = float4(input.pos.xyz, 1.0);
    output.texCoord = input.pos.xy * 0.5 + 0.5;
    return output;
}
