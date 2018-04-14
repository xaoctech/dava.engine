#define EARTH_RADIUS 6360e+3
#define ATMOSPHERE_HEIGHT 60e+3 /* resulting 6420e+3 */
#define ATMOSPHERE_RADIUS (EARTH_RADIUS + ATMOSPHERE_HEIGHT) 
#define REYLEIGH_SCATTERING_CONSTANT float3(5.472e-6, 1.279e-5, 3.121e-5)
#define MIE_SCATTERING_CONSTANT float3(6.299e-6, 9.629e-6, 1.438e-5)
#define OZONE_ABSORPTION_CONSTANT float3(2.0556e-6, 4.9788e-6, 2.136e-7)
#define DEFAULT_SUN_INTENSITY 120000.0
#define ATMOSPHERE_COLOR_BIAS (0.0 * float3(0.005, 0.01, 0.02)) /* furiously simulating multiple scattering with single constant */
#define ENABLE_OUTER_SPACE 1

#ensuredefined PRECOMPUTE_TRANSMITTANCE 0
#ensuredefined PRECOMPUTE_SCATTERING 0

/******************************************************************************
 *
 * Shared functions
 *
 ******************************************************************************/

float atmosphereIntersection(float3 o, float3 v)
{
    float b = dot(v, o);
    float d = b * b - dot(o, o) + ATMOSPHERE_RADIUS * ATMOSPHERE_RADIUS;
    return (d < 0.0) ? -1.0 : (-b + sqrt(d));
}

float3 atmosphereOuterIntersection(float3 origin, float3 direction)
{
    float3 result = 0.0;
    float b = dot(direction, origin);
    float d = b * b - dot(origin, origin) + ATMOSPHERE_RADIUS * ATMOSPHERE_RADIUS;
    float intersectionOccured = step(b, 0.0) * step(0.0, d);
    float ds = sqrt(max(0.0, d));
    return float3(-b - ds, -b + ds, intersectionOccured);
}

float3 planetOuterIntersection(float3 origin, float3 direction)
{
    float3 result = 0.0;
    float b = dot(direction, origin);
    float d = b * b - dot(origin, origin) + EARTH_RADIUS * EARTH_RADIUS;
    float intersectionOccured = step(b, 0.0) * step(0.0, d);
    float ds = sqrt(max(0.0, d));
    return float3(-b - ds, -b + ds, intersectionOccured);
}

float2 OpticalDensityAtHeight(float h)
{
    float2 hScaled = h / float2(7994.0, 1200.0);
    return exp(-hScaled);
}

float2 OpticalLength(float3 from, float3 to, int samples)
{
    float3 directionStep = (to - from) / float(samples);
    float3 direction = from;

    float2 result = 0.0;
    for (int i = 0; i < samples; ++i)
    {
        float height = max(0.0, length(direction) - EARTH_RADIUS);
        result += OpticalDensityAtHeight(height);
        direction += directionStep;
    }
    return result * length(directionStep);
}

float3 Extinction(float3 origin, float3 target, float3 light, float2 phase, float turbidity)
{
    float extinctionM = MIE_SCATTERING_CONSTANT * (1.0 - turbidity);
    float3 extinctionR = OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT;
    float2 opticalLength = OpticalLength(origin, target, ATMOSPHERE_SCATTERING_SAMPLES);
    return exp(-extinctionR * opticalLength.x - extinctionM * opticalLength.y);
}
