#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"
#include "include/ibl.h"

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
    output.position = float4(input.position.xyz, 1.0);

#if (DIFFUSE_SPHERICAL_HARMONICS)
    output.varTexCoord0 = input.position.xy;
#else
    output.varTexCoord0 = input.position.xy * ndcToUvMapping.xy + ndcToUvMapping.zw;
#endif

    return output;
}
