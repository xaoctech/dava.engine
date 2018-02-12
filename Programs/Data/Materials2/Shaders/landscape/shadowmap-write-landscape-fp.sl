#include "include/common.h"

fragment_in
{
    float4 projectedPosition : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

[auto][a] property float4 shadowParameters; // float4(filter radius, cascades blend size, number of cascades, bias)

#define shadowMapSlopeScale 2.0

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float2 dxdy = float2(ddx(input.projectedPosition.z), ddy(input.projectedPosition.z));
    float bias = length(dxdy) * shadowMapSlopeScale + shadowParameters.w;
    output.color = input.projectedPosition.z / input.projectedPosition.w + bias;

    return output;
}
