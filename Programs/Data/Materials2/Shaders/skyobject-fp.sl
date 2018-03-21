#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"

#ensuredefined RGBM_INPUT 0

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
};

fragment_out
{
#if USE_FRAMEBUFFER_FETCH
    float4 color : SV_TARGET4;
#elif (FOWARD_WITH_COMBINE)
    float4 color : SV_TARGET1;
#else
    float4 color : SV_TARGET0;
#endif
};

uniform sampler2D environmentMap;

fragment_out fp_main(fragment_in input)
{
    float4 sampledColor = tex2D(environmentMap, input.varTexCoord0);

#if (RGBM_INPUT)
    sampledColor.xyz = DecodeRGBM(sampledColor);
#endif

    float3 value = sampledColor.xyz * environmentColor.xyz / GLOBAL_LUMINANCE_SCALE;

    fragment_out output;
    output.color = float4(value, 1.0);
    return output;
}
