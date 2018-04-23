uniform sampler2D precomputedTransmittance;

float3 InScattering(float3 origin, float3 target, float3 light, float mieAnisotripy, float turbidity)
{
    float3 scatteringR = REYLEIGH_SCATTERING_CONSTANT;
    float3 extinctionR = OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT;

    float scatteringM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);
    float extinctionM = MIE_SCATTERING_CONSTANT * (1.0 + turbidity);

    float3 positionStep = (target - origin) / float(ATMOSPHERE_SCATTERING_SAMPLES);
    float ds = length(positionStep);

    float3 direction = positionStep / ds;
    float2 phase = ScatteringPhaseFunctions(direction, light, mieAnisotripy);

    float3 iR = 0.0;
    float3 iM = 0.0;
    float2 opticalLengthToOrigin = 0.0;
    for (int i = 0; i < ATMOSPHERE_SCATTERING_SAMPLES; ++i)
    {
        float height = max(0.0, length(origin) - EARTH_RADIUS);
        float2 density = OpticalDensityAtHeight(height) * ds;

        float3 sampledTransmittance = tex2D(precomputedTransmittance, float2(light.z * 0.5 + 0.5, height / ATMOSPHERE_HEIGHT)).xyz;
        float3 computedTransmittance = exp(-extinctionR * opticalLengthToOrigin.x - extinctionM * opticalLengthToOrigin.y);

        float3 transmittance = sampledTransmittance * computedTransmittance;

        iR += transmittance * density.x;
        iM += transmittance * density.y;

        opticalLengthToOrigin += density;
        origin += positionStep;
    }

    return (iR * phase.x * scatteringR + iM * phase.y * scatteringM) / (4.0 * _PI);
}

#if (PRECOMPUTE_SCATTERING)

/******************************************************************************
 *
 * Precompute scattering
 *
 ******************************************************************************/

float4 PrecomputeScattering(float normalizedHeight, float zenithAngleSin, float sunAngleSin)
{
    float3 o = float3(0.0, 0.0, EARTH_RADIUS + normalizedHeight * ATMOSPHERE_HEIGHT);
    float3 v = float3(sqrt(1.0 - saturate(zenithAngleSin * zenithAngleSin)), 0.0, zenithAngleSin);
    float3 l = float3(sqrt(1.0 - saturate(sunAngleSin * sunAngleSin)), 0.0, sunAngleSin);
    float3 t = o + v * atmosphereOuterIntersection(o, v);

    float3 inScattering = InScattering(o, t, l, 0.65, 1.0);

    return float4(inScattering, 1.0);
}

#else

/******************************************************************************
 *
 * Atmosphere sampling function
 *
 ******************************************************************************/

uniform sampler2D precomputedScattering;

float3 SampleAtmosphere(float3 position, float3 v, float3 l, float3 intensity, float t, float g)
{
    float3 result = 0.0;
    
#if (ATMOSPHERE_SCATTERING_SAMPLES > 0)
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

        float3 computedInScattering = InScattering(p0 + t0 * v, p0 + t1 * v, l, g, t);

        /*
        float height = saturate(positionHeight / ATMOSPHERE_HEIGHT);
        float sunAngle = l.z * 0.5 + 0.5;
        float viewAngle = v.z * 0.5 + 0.5;
        float sunDiscreteAngle = floor(sunAngle * 32.0) / 32.0;
        float2 scatteringUv;
        scatteringUv.x = viewAngle / 32.0 + sunDiscreteAngle;
        scatteringUv.y = height;
        float3 sampledInScattering = tex2D(precomputedScattering, scatteringUv);
        result = (intersectsAtmosphere) * (intensity * sampledInScattering);
        */

        float intersectsAtmosphere = lerp(atmosphereHit.z, 1.0, insideAtmosphere);
        result = (intersectsAtmosphere) * (intensity * computedInScattering);
    }
    #else
    {
        float t1 = min(8.0 * ATMOSPHERE_HEIGHT, atmosphereIntersection(p0, v));
        result = intensity * InScattering(p0, p0 + t1 * v, l, g, t) + ATMOSPHERE_COLOR_BIAS;
    }
    #endif

#else
    {
        result = intensity / 4.8; /* resulting 25000 with default Sun illuminance 120000 */
    }
#endif

    result += 0.00001 * tex2D(precomputedScattering, float2(0.5, 0.5));

    return result;
}

#endif
