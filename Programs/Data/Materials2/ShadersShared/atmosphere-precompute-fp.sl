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

[material][a] float layersCount = 32.0;

#if (PRECOMPUTE_TRANSMITTANCE)

float4 PrecomputeTransmittance(float normalizedHeight, float zenithAngleSin)
{
    float3 origin = float3(0.0, 0.0, EARTH_RADIUS + normalizedHeight * ATMOSPHERE_HEIGHT);
    float3 view = float3(sqrt(1.0 - saturate(zenithAngleSin * zenithAngleSin)), 0.0, zenithAngleSin);
    
    float3 atmosphereIntersection = origin + atmosphereIntersection(origin, view) * view;
    float2 opticalLength = OpticalLength(atmosphereIntersection, origin, 256);
    
    float3 tR = exp(-opticalLength.x * (OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT));
    float tM = exp(-opticalLength.y * MIE_SCATTERING_CONSTANT);
    
    return float4(tR * tM, 1.0);
}

#else

#include "include/atmosphere.h"

#endif

fragment_out fp_main(fragment_in input)
{
    fragment_out output;
    
    float height = input.uv.y;
    
#if (PRECOMPUTE_TRANSMITTANCE)
    {
        float viewAngle = input.uv.x * 2.0 - 1.0;
        output.color = PrecomputeTransmittance(height, viewAngle);
    }
#elif (PRECOMPUTE_SCATTERING)
    {
        float viewAngle = frac(input.uv.x * layersCount) * 2.0 - 1.0;
        float sunAngle = (floor(input.uv.x * layersCount) / layersCount) * 2.0 - 1.0;
        output.color = PrecomputeScattering(height, viewAngle, sunAngle);
    }
#endif
    return output;
}
