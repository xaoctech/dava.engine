#include "include/common.h"
#include "include/shading-options.h"
#include "include/all-input.h"
#include "include/math.h"
#include "include/atmosphere-helpers.h"

fragment_in
{
    float2 uv : TEXCOORD0;
};

fragment_out
{
    float4 color : SV_TARGET0;
};

#if (PRECOMPUTE_TRANSMITTANCE == 0)

#include "include/atmosphere.h"
[material][a] property float layersCount = 1.0;

#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    
#if (PRECOMPUTE_TRANSMITTANCE)
    {
        float lightAngle = TexCoordToLightAngle(input.uv.x);
        float height = TexCoordToHeightAboveGround(input.uv.y);
        float3 transmittance = PrecomputeTransmittance(height, lightAngle);
        output.color = float4(transmittance, 1.0);
    }
#elif (PRECOMPUTE_SCATTERING)
    {
        float u = input.uv.x; // frac(input.uv.x * layersCount);
        float v = input.uv.y;
        float w = 0.0; // floor(input.uv.x * layersCount) / layersCount;

        float a_height = DEFAULT_HEIGHT_ABOVE_GROUND;
        float a_viewAngle = TexCoordToViewAngle(u, a_height);
        float a_sunAngle = TexCoordToLightAngle(v);

        output.color = PrecomputeScattering(a_height, a_viewAngle, a_sunAngle);
    }
#endif
    return output;
}
