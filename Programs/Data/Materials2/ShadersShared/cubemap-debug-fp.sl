#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/structures.h"
#include "include/math.h"

fragment_in
{
    float2 varTexCoord0 : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

#ensuredefined DISPLAY_SPHERICAL_HARMONICS 0
#ensuredefined DISPLAY_INDIRECT_LOOKUP 0

#if (DISPLAY_INDIRECT_LOOKUP || DISPLAY_SPHERICAL_HARMONICS)
// nothing to do here
#else
uniform samplerCUBE environmentMap;
[material][a] property float sampledLevel = 0.0;
#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

#if (DISPLAY_INDIRECT_LOOKUP)

    output.color = tex2D(indirectSpecularLookup, input.varTexCoord0 * 0.5 + 0.5);

#elif (DISPLAY_SPHERICAL_HARMONICS)

    float3 direction = WrapEquirectangularToDirection(input.varTexCoord0);
    output.color = 0.0;
    {
        output.color += sphericalHarmonics[0] * (0.282095);
        output.color += sphericalHarmonics[1] * (0.488603 * direction.y);
        output.color += sphericalHarmonics[2] * (0.488603 * direction.z);
        output.color += sphericalHarmonics[3] * (0.488603 * direction.x);
        output.color += sphericalHarmonics[4] * (1.092548 * direction.x * direction.y);
        output.color += sphericalHarmonics[5] * (1.092548 * direction.y * direction.z);
        output.color += sphericalHarmonics[6] * (0.315392 * (3.0 * direction.z * direction.z - 1.0));
        output.color += sphericalHarmonics[7] * (1.092548 * direction.x * direction.z);
        output.color += sphericalHarmonics[8] * (0.546274 * (direction.x * direction.x - direction.y * direction.y));
    }
    output.color.xyz = LinearTosRGB(output.color.xyz);

#else

    float3 direction = WrapEquirectangularToDirection(input.varTexCoord0);
    output.color = texCUBElod(environmentMap, direction, sampledLevel);
    output.color.xyz = LinearTosRGB(output.color.xyz);

#endif

    if (isNan(output.color.r) || isNan(output.color.g) || isNan(output.color.b))
        output.color.xyz = float3(0.0, 0.0, 1.0);

    return output;
}
