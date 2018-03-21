#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D environmentMap;

fragment_out fp_main(fragment_in input)
{
    float3 value = environmentColor.xyz / GLOBAL_LUMINANCE_SCALE;
    value *= tex2D(environmentMap, input.varTexCoord0).xyz;

#if (!IB_REFLECTIONS_PREPARE)
    value = PerformToneMapping(value, cameraDynamicRange, 1.0);
#endif

    fragment_out output;
    output.color = float4(value, 1.0);
    return output;
}
