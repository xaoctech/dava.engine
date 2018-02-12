#include <Render/Highlevel/Atmosphere.h>

namespace DAVA
{
const uint32 inScatteringSamples = 16;
const uint32 outScatteringSamples = 16;

const float EARTH_RADIUS = 6360e+3f;
const float atmosphereHeight = 60e+3f;
const float Ra(EARTH_RADIUS + atmosphereHeight);
const Vector2 H0 = Vector2(7994.0f, 1200.0f);
const Vector3 betaR = Vector3(5.472e-6f, 1.279e-5f, 3.121e-5f);
const Vector3 betaM = Vector3(6.299e-6f, 9.629e-6f, 1.438e-5f);

const Vector3 Atmosphere::GroundLevel = Vector3(0.0, 0.0, EARTH_RADIUS);

float Atmosphere::PhaseFunctionRayleigh(float cosine)
{
    return (3.0f / 4.0f) * (1.0f + cosine * cosine) / (4.0f * PI);
}

float Atmosphere::PhaseFunctionMie(float cosine, float anisotropy)
{
    float asquared = anisotropy * anisotropy;
    float t1 = 3.0f * (1.0f - asquared) / 2.0f * (2.0f + asquared);
    float t2 = (1.0f + cosine * cosine) / std::pow(1.0f + asquared - 2.0f * anisotropy * cosine, 3.0f / 2.0f);
    return t1 * t2 / (4.0f * PI);
}

Vector2 Atmosphere::OpticalDensity(const Vector3& point)
{
    float heightAboveGround = std::max(0.0f, point.Length() - EARTH_RADIUS);
    return Vector2(std::exp(-heightAboveGround / H0.x), std::exp(-heightAboveGround / H0.y));
}

Vector2 Atmosphere::OpticalDensity(const Vector3& start, const Vector3& direction)
{
    Vector3 dp = direction / static_cast<float>(outScatteringSamples);
    Vector3 origin = start + 0.5f * dp;

    Vector2 result = Vector2(0.0f, 0.0f);
    for (int i = 0; i < outScatteringSamples; ++i)
    {
        result += OpticalDensity(origin);
        origin += dp;
    }
    return result * dp.Length();
}

float Atmosphere::IntersectAtmosphere(const Vector3& origin, const Vector3& direction)
{
    float b = direction.DotProduct(origin);
    float d = b * b - origin.DotProduct(origin) + Ra * Ra;
    return (d < 0.0f) ? 0.0f : (-b + sqrt(d));
}

Vector2 Atmosphere::ComputePhaseFunctions(const Vector3& a, const Vector3& b, float anisotropy)
{
    float cosine = a.DotProduct(b);
    Vector2 result;
    result.x = PhaseFunctionRayleigh(cosine);
    result.y = PhaseFunctionMie(cosine, anisotropy);
    return result;
}

Vector3 Atmosphere::ComputeInScattering(const Vector3& origin, const Vector3& target, const Vector3& light, const Vector2& phaseFunctions, const Parameters& parametes)
{
    Vector3 scaledBetaM = betaM * (1.0f + parametes.turbidity);
    Vector3 positionStep = (target - origin) / static_cast<float>(inScatteringSamples);

    Vector3 resultR = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 resultM = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 pos = origin + 0.5f * positionStep;

    for (int i = 0; i < inScatteringSamples; ++i)
    {
        Vector2 inScattering = OpticalDensity(pos);
        Vector2 opticalDepth = OpticalDensity(pos, light * IntersectAtmosphere(pos, light)) + OpticalDensity(pos, origin - pos);
        Vector3 outScattering = -betaR * opticalDepth.x - scaledBetaM * opticalDepth.y;
        outScattering.x = std::exp(outScattering.x);
        outScattering.y = std::exp(outScattering.y);
        outScattering.z = std::exp(outScattering.z);
        resultR += inScattering.x * outScattering;
        resultM += inScattering.y * outScattering;
        pos += positionStep;
    }

    return (resultR * betaR * phaseFunctions.x + resultM * scaledBetaM * phaseFunctions.y) * positionStep.Length();
}

Vector3 Atmosphere::ComputeOutScattering(const Vector3& origin, const Vector3& target, const Parameters& parametes)
{
    Vector3 scaledBetaM = betaM * (1.0f + parametes.turbidity);
    Vector2 opticalDepth = OpticalDensity(origin, target - origin);

    Vector3 e = -betaR * opticalDepth.x - scaledBetaM * opticalDepth.y;
    e.x = std::exp(e.x);
    e.y = std::exp(e.y);
    e.z = std::exp(e.z);
    return e;
}
}
