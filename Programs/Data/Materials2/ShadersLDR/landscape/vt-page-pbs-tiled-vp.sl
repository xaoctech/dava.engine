#define LDR_FLOW 1

#include "include/common.h"
#include "include/shading-options.h"

#ensuredefined USE_PREVIOUS_LANDSCAPE_LAYER 0

vertex_in
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

vertex_out
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float2 texCoordTiled0 : TEXCOORD1;
    float2 texCoordTiled1 : TEXCOORD2;
    float2 texCoordTiled2 : TEXCOORD3;
    float2 texCoordTiled3 : TEXCOORD4;
#if LANDSCAPE_VT_PAGE || USE_PREVIOUS_LANDSCAPE_LAYER
    float2 pageTexCoord : TEXCOORD5;
#endif
};

[material][instance] property float2 tiling0 = float2(60, 60);
[material][instance] property float2 tiling1 = float2(60, 60);
[material][instance] property float2 tiling2 = float2(60, 60);
[material][instance] property float2 tiling3 = float2(60, 60);

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    output.pos = float4(input.pos.xyz, 1.0);
    output.texCoord = float2(input.uv.x, 1.0 - input.uv.y);
    output.texCoordTiled0 = output.texCoord * tiling0.xy;
    output.texCoordTiled1 = output.texCoord * tiling1.xy;
    output.texCoordTiled2 = output.texCoord * tiling2.xy;
    output.texCoordTiled3 = output.texCoord * tiling3.xy;

#if LANDSCAPE_VT_PAGE || USE_PREVIOUS_LANDSCAPE_LAYER
    output.pageTexCoord = input.pos.xy * 0.5 + 0.5;
#endif

    return output;
}
