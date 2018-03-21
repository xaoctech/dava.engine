#define EARTH_RADIUS 6360e+3
#define ATMOSPHERE_HEIGHT 60e+3 /* resulting 6420e+3 */
#define ATMOSPHERE_RADIUS (EARTH_RADIUS + ATMOSPHERE_HEIGHT) 
#define REYLEIGH_SCATTERING_CONSTANT float3(5.472e-6, 1.279e-5, 3.121e-5)
#define MIE_SCATTERING_CONSTANT float3(6.299e-6, 9.629e-6, 1.438e-5)
#define OZONE_ABSORPTION_CONSTANT float3(2.0556e-6, 4.9788e-6, 2.136e-7)
#define DEFAULT_SUN_INTENSITY 120000.0
#define ATMOSPHERE_COLOR_BIAS float3(0.005, 0.01, 0.02) /* furiously simulating multiple scattering with single constant */
#define ENABLE_OUTER_SPACE 0

float atmosphereIntersection(float3 origin, float3 direction)
{
    float b = dot(direction, origin);
    float d = b * b - dot(origin, origin) + ATMOSPHERE_RADIUS * ATMOSPHERE_RADIUS;
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

float2 OpticalDensity(float3 pos)
{
    float h = max(0.0, length(pos) - EARTH_RADIUS);
    return exp(-h / float2(7994.0, 1200.0));
}

float2 OpticalLength(float3 from, float3 to)
{
    float3 directionStep = (to - from) / float(ATMOSPHERE_SCATTERING_SAMPLES);

    float2 result = 0.0;
    for (int i = 0; i < ATMOSPHERE_SCATTERING_SAMPLES; ++i)
    {
        result += OpticalDensity(from);
        from += directionStep;
    }
    return result * length(directionStep);
}

float3 InScattering(float3 origin, float3 target, float3 light, float2 phase, float turbidity)
{
    float3 scatteringR = REYLEIGH_SCATTERING_CONSTANT;
    float3 scatteringM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);
    float3 extinctionR = OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT;
    float3 extinctionM = scatteringM;

    float3 positionStep = (target - origin) / float(ATMOSPHERE_SCATTERING_SAMPLES);
    float ds = length(positionStep);

    float3 pos = origin;
    float2 opticalLengthToOrigin = 0.0;

    float3 iR = 0.0;
    float3 iM = 0.0;
    for (int i = 0; i < ATMOSPHERE_SCATTERING_SAMPLES; ++i)
    {
        float2 density = OpticalDensity(pos);

        float2 opticalLengthToLight = OpticalLength(pos + light * atmosphereIntersection(pos, light), pos);
        float3 transmittance = exp(-extinctionR * (opticalLengthToOrigin.x + opticalLengthToLight.x) - extinctionM * (opticalLengthToOrigin.y + opticalLengthToLight.y));

        iR += density.x * transmittance;
        iM += density.y * transmittance;

        opticalLengthToOrigin += density * ds;
        pos += positionStep;
    }

    return (iR * phase.x * scatteringR + iM * phase.y * scatteringM) * (ds / (4.0 * _PI));
}

float3 Extinction(float3 origin, float3 target, float3 light, float2 phase, float turbidity)
{
    float2 opticalLength = OpticalLength(origin, target);
    float3 extinctionM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);
    float3 extinctionR = OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT;
    return exp(-extinctionR * opticalLength.x - extinctionM * opticalLength.y);
}

float3 SampleAtmosphere(float3 position, float3 v, float3 l, float3 intensity, float t, float g)
{
    float3 result = 0.0;

#if (ATMOSPHERE_SCATTERING_SAMPLES > 0)
    float2 ph = ScatteringPhaseFunctions(v, l, g);

    float positionHeight = position.z;
    float3 p0 = float3(0.0, 0.0, positionHeight + EARTH_RADIUS);

    #if (ENABLE_OUTER_SPACE)
    {
        float3 planetHit = planetOuterIntersection(p0, v);
        float3 atmosphereHit = atmosphereOuterIntersection(p0, v);

        float t0 = max(0.0, atmosphereHit.x);

        float planetNearHit = min(planetHit.x, planetHit.y);
        float tInner = lerp(atmosphereHit.y, min(atmosphereHit.y, planetNearHit), planetHit.z);
        float tOuter = min(planetNearHit, atmosphereHit.y);

        float insideAtmosphere = step(positionHeight, ATMOSPHERE_HEIGHT);
        float t1 = lerp(tOuter, tInner, insideAtmosphere);

        float intersectsAtmosphere = lerp(atmosphereHit.z, 1.0, insideAtmosphere);
        result = (intersectsAtmosphere) * (intensity * InScattering(p0 + t0 * v, p0 + t1 * v, l, ph, t));
    }
    #else
    {
        float t1 = min(8.0 * ATMOSPHERE_HEIGHT, atmosphereIntersection(p0, v));
        result = intensity * InScattering(p0, p0 + t1 * v, l, ph, t) + ATMOSPHERE_COLOR_BIAS;
    }
    #endif

#else
    result = intensity / 4.8; /* resulting 25000 with default Sun illuminance 120000 */
#endif

    return result;
}
