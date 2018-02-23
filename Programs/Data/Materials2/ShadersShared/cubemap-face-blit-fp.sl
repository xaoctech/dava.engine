#include "include/common.h"

fragment_in
{
    float3 texCoord : TEXCOORD0;
};

fragment_out
{
    float4 target0 : SV_TARGET0;
};

uniform samplerCUBE texture0;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    output.target0 = texCUBE(texture0, input.texCoord);
    return output;
}