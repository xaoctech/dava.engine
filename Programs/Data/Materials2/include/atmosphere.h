// #ensuredefined ATMOSPHERE_SCATTERING_SAMPLES   8

#define EARTH_RADIUS 6360e+3
#define ATMOSPHERE_HEIGHT 60e+3 /* resulting 6420e+3 */
#define ATMOSPHERE_RADIUS (EARTH_RADIUS + ATMOSPHERE_HEIGHT) 
#define REYLEIGH_SCATTERING_CONSTANT float3(5.472e-6, 1.279e-5, 3.121e-5)
#define MIE_SCATTERING_CONSTANT float3(6.299e-6, 9.629e-6, 1.438e-5)

float atmosphereIntersection(float3 origin, float3 direction)
{
    float b = dot(direction, origin);
    float d = b * b - dot(origin, origin) + ATMOSPHERE_RADIUS * ATMOSPHERE_RADIUS;
    return (d < 0.0) ? -1.0 : (-b + sqrt(d));
}

float2 DensityAtPosition(float3 pos)
{
    float h = max(0.0, length(pos) - EARTH_RADIUS);
    return exp(-h / float2(7994.0, 1200.0));
}

float2 OpticalDensity(float3 start, float3 dir)
{
    float3 dp = dir / float(ATMOSPHERE_SCATTERING_SAMPLES);
    float3 origin = start + 0.5 * dp;

    float2 result = 0.0;
    for (int i = 0; i < ATMOSPHERE_SCATTERING_SAMPLES; ++i)
    {
        result += DensityAtPosition(origin);
        origin += dp;
    }
    return result * length(dp);
}

float3 InScattering(float3 origin, float3 target, float3 light, float2 phase, float turbidity)
{
    float3 scaledBetaM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);
    float3 positionStep = (target - origin) / float(ATMOSPHERE_SCATTERING_SAMPLES);

    float3 resultR = 0.0;
    float3 resultM = 0.0;
    float3 pos = origin + 0.5 * positionStep;

    for (int i = 0; i < ATMOSPHERE_SCATTERING_SAMPLES; ++i)
    {
        float2 inScattering = DensityAtPosition(pos);
        float2 opticalDepth = OpticalDensity(pos, light * atmosphereIntersection(pos, light)) + OpticalDensity(pos, origin - pos);
        float3 outScattering = exp(-REYLEIGH_SCATTERING_CONSTANT * opticalDepth.x - scaledBetaM * opticalDepth.y);
        resultR += inScattering.x * outScattering;
        resultM += inScattering.y * outScattering;
        pos += positionStep;
    }

    return (resultR * REYLEIGH_SCATTERING_CONSTANT * phase.x + resultM * scaledBetaM * phase.y) * length(positionStep);
}

float3 OutScattering(float3 origin, float3 target, float3 light, float2 phase, float turbidity)
{
    float3 scaledBetaM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);
    float2 opticalDepth = OpticalDensity(origin, target - origin);
    return exp(-REYLEIGH_SCATTERING_CONSTANT * opticalDepth.x - scaledBetaM * opticalDepth.y);
}

float3 SampleAtmosphere(float height, float3 v, float3 l, float3 lightColor, float t, float g)
{
    float3 bias = float3(0.033, 0.05, 0.075); /* furiously simulating multiple scattering with single constant */
    float3 planetPos = float3(0.0, 0.0, EARTH_RADIUS + height);

    float2 ph = ScatteringPhaseFunctions(v, l, g);
    float a = min(4.0 * ATMOSPHERE_HEIGHT, atmosphereIntersection(planetPos, v));
    float3 result = lightColor * InScattering(planetPos, planetPos + a * v, l, ph, t) + bias;

    return result;
}
