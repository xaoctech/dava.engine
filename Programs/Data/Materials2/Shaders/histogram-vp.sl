#include "include/common.h"
#include "include/shading-options.h"
#include "include/structures.h"
#include "include/math.h"

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
};

uniform sampler2D srcTexture;

[material][a] property float2 srcTexSize;
[material][a] property float bucketsCount;

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float2 texCoord = input.position.xy / srcTexSize;
    float luminance = RGBtoLuminance(tex2D(srcTexture, texCoord).rgb);
    float index = log2(1 + luminance) * bucketsCount;
    output.position = float4(index / bucketsCount * 2.0 - 1.0, 0.5, 1.0, 1.0);
    return output;
}