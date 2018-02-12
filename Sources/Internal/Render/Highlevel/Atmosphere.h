#pragma once

#include <Math/Vector.h>

namespace DAVA
{
class Atmosphere
{
public:
    static const Vector3 GroundLevel;

public:
    struct Parameters
    {
        float turbidity = 0.0f;
        float scatteringAnisotropy = 0.0f;
    };

    static float PhaseFunctionRayleigh(float cosine);
    static float PhaseFunctionMie(float cosine, float anisotropy);
    static float IntersectAtmosphere(const Vector3& origin, const Vector3& direction);
    static Vector2 OpticalDensity(const Vector3& point);
    static Vector2 OpticalDensity(const Vector3& start, const Vector3& direction);
    static Vector2 ComputePhaseFunctions(const Vector3& a, const Vector3& b, float anisotropy);
    static Vector3 ComputeInScattering(const Vector3& origin, const Vector3& target, const Vector3& light, const Vector2& phaseFunctions, const Parameters& parametes);
    static Vector3 ComputeOutScattering(const Vector3& origin, const Vector3& target, const Parameters& parametes);
};
}
