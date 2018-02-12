#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

vertex_in
{
    float3 position : POSITION;
    float2 texcoord0 : TEXCOORD0;
};

vertex_out
{
    float4 position : SV_POSITION;
    float2 varTexCoord0 : TEXCOORD0;
};

[auto][a] property float4x4 worldViewProjMatrix;

vertex_out vp_main(vertex_in input)
{
    float4x4 mwpWOtranslate = float4x4(worldViewProjMatrix[0], worldViewProjMatrix[1], worldViewProjMatrix[2], float4(0.0, 0.0, 0.0, 0.0));
    float4 vecPos = mul(float4(input.position.xyz, 1.0), mwpWOtranslate);

    vertex_out output;
    output.position = float4(vecPos.x, vecPos.y, distantDepthValue * vecPos.w, vecPos.w);
    output.varTexCoord0 = input.texcoord0;
    return output;
}
