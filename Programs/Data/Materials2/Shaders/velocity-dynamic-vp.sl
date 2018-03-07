#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"

[auto][global] property float4x4 prevViewProjMatrix;

vertex_in
{
    float3 position : POSITION;
};

vertex_out
{
    float4 position : SV_POSITION;
    float4 currPosition : TEXCOORD0;
    float4 prevPosition : TEXCOORD1;
    float2 scale : TEXCOORD2;
};

vertex_out vp_main(vertex_in input)
{
    vertex_out output;

    float4 inputPosition = float4(input.position.xyz, 1.0f);
    float4 worldPositionCurrent = mul(inputPosition, worldMatrix);
    float4 worldPositionPrev = mul(inputPosition, prevWorldMatrix);

    float4 currPos = mul(worldPositionCurrent, viewProjMatrix);
    output.position = currPos;
    output.currPosition = currPos;

    output.prevPosition = mul(worldPositionPrev, prevViewProjMatrix);

    output.scale = viewportSize / renderTargetSize;

    return output;
}
