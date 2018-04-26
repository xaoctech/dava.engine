uniform sampler2D precomputedTransmittance;
uniform sampler2D precomputedScattering;

float ModifiedRayleighPhaseFunction(in float cs)
{
    return (8.0 / 10.0) * (7.0 / 5.0 + 1.0 / 2.0 * cs);
}

float4 IntegrateInScattering(float3 origin, float3 target, float3 light, int samples)
{
    float3 extinctionR = (OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT);
    float3 scatteringR = REYLEIGH_SCATTERING_CONSTANT;

    float3 extinctionM = MIE_SCATTERING_CONSTANT;
    float3 scatteringM = MIE_SCATTERING_CONSTANT / 0.9;

    float3 positionStep = (target - origin) / float(samples);
    float ds = length(positionStep);

    float lightDirectionTexCoord = LightAngleToTexCoord(light.z);

    float4 integral = 0.0;
    float2 opticalLengthToOrigin = 0.0;
    float3 samplePosition = origin;

    for (int i = 0; i < samples; ++i)
    {
        float height = max(0.0, length(samplePosition) - EARTH_RADIUS);

        float heightTexCoord = HeightAboveGroundToTexCoord(height);
        float3 sampledTransmittance = tex2D(precomputedTransmittance, float2(lightDirectionTexCoord, heightTexCoord)).xyz;

        float3 computedTransmittance = exp(-extinctionR * opticalLengthToOrigin.x - extinctionM * opticalLengthToOrigin.y);
        float3 totalTransmittance = sampledTransmittance * computedTransmittance;

        float2 density = OpticalDensityAtHeight(height) * ds;

        integral += float4(totalTransmittance.xyz * density.x, totalTransmittance.x * density.y);

        opticalLengthToOrigin += density;
        samplePosition += positionStep;
    }

    return float4(integral.xyz * scatteringR, integral.w * scatteringM.x);
}

/******************************************************************************
 *
 * Precompute scattering
 *
 ******************************************************************************/

float4 PrecomputeScattering(float heightAboveGround, float viewAngle, float lightAngle)
{
    float3 a_origin = float3(0.0, 0.0, EARTH_RADIUS + heightAboveGround);
    float3 a_light = float3(sqrt(1.0 - saturate(lightAngle * lightAngle)), 0.0, lightAngle);

    float3 a_view = float3(sqrt(1.0 - saturate(viewAngle * viewAngle)), 0.0, viewAngle);
    float3 a_target = a_origin + a_view * min(8.0 * ATMOSPHERE_HEIGHT, atmosphereIntersection(a_origin, a_view));

    return IntegrateInScattering(a_origin, a_target, a_light, 512);
}

/******************************************************************************
 *
 * Atmosphere sampling function
 *
 ******************************************************************************/

float3 EvaluateSingleScattering(float4 integral, float3 in_view, float3 in_light, float in_mieAnisotripy)
{
    float3 lightIntensityScale = (DEFAULT_SUN_INTENSITY / GLOBAL_LUMINANCE_SCALE) / tex2D(precomputedTransmittance, float2(1.0, 0.0)).xyz;

    float cosTheta = dot(in_view, in_light);
    float phaseR = ModifiedRayleighPhaseFunction(cosTheta);
    float phaseM = PhaseFunctionHenyeyGreenstein(cosTheta, in_mieAnisotripy);

    float3 iR = integral.xyz;
    float3 iM = approximateMieScatteringFromRayleigh(iR, integral.w);

    return lightIntensityScale * (iR * phaseR + iM * phaseM) / (4.0 * _PI);
}

float3 InScattering(float3 in_origin, float3 in_target, float3 light_in, float mieAnisotripy_in)
{
    float3 a_direction = normalize(in_target - in_origin);
    float4 a_integral = IntegrateInScattering(in_origin, in_target, light_in, ATMOSPHERE_SCATTERING_SAMPLES);
    return EvaluateSingleScattering(a_integral, a_direction, light_in, mieAnisotripy_in);
}

float3 Extinction(float3 origin, float3 target)
{
    float3 extinctionM = MIE_SCATTERING_CONSTANT;
    float3 extinctionR = OZONE_ABSORPTION_CONSTANT + REYLEIGH_SCATTERING_CONSTANT;
    float2 opticalLength = OpticalLength(origin, target, ATMOSPHERE_SCATTERING_SAMPLES);
    return exp(-extinctionR * opticalLength.x - extinctionM * opticalLength.y);
}

float3 SampleAtmosphere(float3 position, float3 v, float3 l, float3 intensity, float t, float g)
{
    // float3 a_view = float3(sqrt(1.0 - saturate(v.z * v.z)), 0.0, v.z);
    // float3 a_light = float3(sqrt(1.0 - saturate(l.z * l.z)), 0.0, l.z);
    // float3 a_target = a_origin + a_view * min(8.0 * ATMOSPHERE_HEIGHT, atmosphereIntersection(a_origin, a_view));
    // float4 a_integral = IntegrateInScattering(a_origin, a_target, a_light, t);
    // result = EvaluateSingleScattering(a_integral, v, l, g);

    float2 sampleCoords;
    sampleCoords.x = ViewAngleToTexCoord(v.z, DEFAULT_HEIGHT_ABOVE_GROUND);
    sampleCoords.y = LightAngleToTexCoord(l.z);

    float4 sampledIntegral = tex2D(precomputedScattering, sampleCoords);
    float3 singleScattering = EvaluateSingleScattering(sampledIntegral, v, l, g);

    return singleScattering;
}

