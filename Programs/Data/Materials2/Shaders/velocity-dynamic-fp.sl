#include "include/common.h"

color_mask = rg;

fragment_in
{
    float4 position : SV_POSITION;
    float4 currPosition : TEXCOORD0;
    float4 prevPosition : TEXCOORD1;
    float2 scale : TEXCOORD2;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

[auto][global] property float4 cameraProjJitterPrevCurr;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float4 prevPosition = input.prevPosition / input.prevPosition.w;
    prevPosition.xy += cameraProjJitterPrevCurr.xy;

    float4 currPosition = input.currPosition / input.currPosition.w;
    currPosition.xy += cameraProjJitterPrevCurr.zw;

    float2 vel = (prevPosition.xy - currPosition.xy) * 0.5f;

    output.color = float4(vel * input.scale, 0.0f, 0.0f);
    return output;
}