#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"

blending
{
    src = src_alpha dst = inv_src_alpha
}

fragment_in
{
    float4 color : TEXCOORD0;
};

fragment_out
{
    float4 target0 : SV_TARGET0;
};

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    output.target0 = input.color;

    return output;
}
