#define EARTH_RADIUS 6360e+3
#define ATMOSPHERE_HEIGHT 80e+3
#define ATMOSPHERE_RADIUS (EARTH_RADIUS + ATMOSPHERE_HEIGHT) 

#define REYLEIGH_SCATTERING_CONSTANT float3(6.554e-6, 1.428e-5, 2.853e-5) // float3(5.472e-6, 1.279e-5, 3.121e-5)
#define MIE_SCATTERING_CONSTANT float3(6.894e-6, 1.018e-5, 1.438e-5) // float3(6.299e-6, 9.629e-6, 1.438e-5)
#define OZONE_ABSORPTION_CONSTANT float3(3.426 * 6.0e-7, 8.298 * 6.0e-7, 0.356 * 6.0e-7)

#define DEFAULT_SUN_INTENSITY 120000.0
#define ATMOSPHERE_COLOR_BIAS (0.0 * float3(0.005, 0.01, 0.02)) /* furiously simulating multiple scattering with single constant */
#define DEFAULT_HEIGHT_ABOVE_GROUND 50.0

#define ENABLE_OUTER_SPACE 0
#ensuredefined PRECOMPUTE_TRANSMITTANCE 0
#ensuredefined PRECOMPUTE_SCATTERING 0

/******************************************************************************
 *
 * Shared functions
 *
 ******************************************************************************/

float atmosphereIntersection(float3 o, float3 dir)
{
    float b = dot(dir, o);
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

#define LIGHT_ANGLE_SCALE 5.3496235 // tan(1.26 * 1.1)

float LightAngleToTexCoord(float a)
{
    return 0.5 * (atan(max(a, -0.1975) * LIGHT_ANGLE_SCALE) / 1.1 + (1.0 - 0.26));
}

float TexCoordToLightAngle(float texCoord)
{
    float val = 1.1 * (2.0 * texCoord - (1.0 - 0.26));
    return (sin(val) / cos(val)) / LIGHT_ANGLE_SCALE;
}

float HeightAboveGroundToTexCoord(float h)
{
    return sqrt(saturate(h / ATMOSPHERE_HEIGHT));
}

float TexCoordToHeightAboveGround(float texCoord)
{
    return texCoord * texCoord * ATMOSPHERE_HEIGHT;
}

float HorizonAngleAtHeight(float a_height)
{
    a_height = max(0.0, a_height);
    return -sqrt(a_height * (2.0 * EARTH_RADIUS + a_height)) / (EARTH_RADIUS + a_height);
}

float TexCoordToViewAngle(float coordinate, float height)
{
    float horizonAngle = HorizonAngleAtHeight(height);

    float viewZenithAngle = 0.0;
    if (coordinate > 0.5)
    {
        viewZenithAngle = pow(coordinate * 2.0 - 1.0, 5.0) * (1.0 - horizonAngle) + horizonAngle;
    }
    else
    {
        viewZenithAngle = horizonAngle - pow(coordinate * 2.0, 5.0) * (horizonAngle + 1.0);
    }
    return viewZenithAngle;
}

float ViewAngleToTexCoord(float viewZenithAngle, float height)
{
    float horizonAngle = HorizonAngleAtHeight(height);

    float coordinate = 0.0;
    if (viewZenithAngle > horizonAngle)
    {
        coordinate = pow(saturate((viewZenithAngle - horizonAngle) / (1.0 - horizonAngle)), 0.2) * 0.5 + 0.5;
    }
    else
    {
        coordinate = pow(saturate((horizonAngle - viewZenithAngle) / (horizonAngle + 1.0)), 0.2) * 0.5;
    }
    return coordinate;
}

float3 approximateMieScatteringFromRayleigh(float3 integralR, in float mieR)
{
    float s0 = mieR / integralR.x;
    float s1 = REYLEIGH_SCATTERING_CONSTANT.x / MIE_SCATTERING_CONSTANT.x;
    float3 s2 = MIE_SCATTERING_CONSTANT / REYLEIGH_SCATTERING_CONSTANT;
    return (s0 * s1) * (s2 * integralR);
}

float3 PrecomputeTransmittance(float heightAboveGround, float zenithAngleSin)
{
    float3 origin = float3(0.0, 0.0, EARTH_RADIUS + heightAboveGround);
    float3 view = float3(sqrt(1.0 - saturate(zenithAngleSin * zenithAngleSin)), 0.0, zenithAngleSin);

    float3 atmosphereIntersection = origin + atmosphereIntersection(origin, view) * view;
    float2 opticalLength = OpticalLength(atmosphereIntersection, origin, 512);

    return exp(-opticalLength.x * (OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT) - opticalLength.y * (MIE_SCATTERING_CONSTANT));
}
