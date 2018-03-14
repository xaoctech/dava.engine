#include "Particles/ParticlesRandom.h"

// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/

namespace DAVA
{
namespace ParticlesRandom
{
float32 HammersleyRnd(uint32 n)
{
    float32 base = 0.5f;
    float32 res = 0.0f;
    while (n != 0)
    {
        if ((n & 1) != 0)
            res += base;
        n >>= 1;
        base *= 0.5f;
    }
    return res;
}

float32 HammersleyRnd(float32 min, float32 max, uint32 n)
{
    return (max - min) * HammersleyRnd(n) + min;
}

float32 VanDerCorputRnd(uint32 n, uint32 base)
{
    float32 res = 0.0f;
    float32 denom = static_cast<float32>(base);
    while (n > 0)
    {
        uint32 mult = n % base;
        res += static_cast<float32>(mult) / denom;
        n /= base;
        denom *= base;
    }
    return res;
}

float32 VanDerCorputRnd(float32 min, float32 max, uint32 n, uint32 base)
{
    return (max - min) * VanDerCorputRnd(n, base) + min;
}

Vector2 SobolSequenceV2Prebult(uint32 index)
{
    static const Vector2 seq[]
    {
      Vector2(-1.0f, -1.0f),
      Vector2(0.0f, 0.0f),
      Vector2(-0.5f, 0.5f),
      Vector2(0.5f, -0.5f),
      Vector2(-0.75f, 0.25f),
      Vector2(0.25f, -0.75f),
      Vector2(-0.25f, -0.25f),
      Vector2(0.75f, 0.75f),
      Vector2(-0.875f, 0.875f),
      Vector2(0.125f, -0.125f),
      Vector2(-0.375f, -0.625f),
      Vector2(0.625f, 0.375f),
      Vector2(-0.625f, -0.375f),
      Vector2(0.375f, 0.625f),
      Vector2(-0.125f, 0.125f),
      Vector2(0.875f, -0.875f),
      Vector2(-0.9375f, 0.0625f),
      Vector2(0.0625f, -0.9375f),
      Vector2(-0.4375f, -0.4375f),
      Vector2(0.5625f, 0.5625f),
      Vector2(-0.6875f, -0.6875f),
      Vector2(0.3125f, 0.3125f),
      Vector2(-0.1875f, 0.8125f),
      Vector2(0.8125f, -0.1875f),
      Vector2(-0.8125f, -0.0625f),
      Vector2(0.1875f, 0.9375f),
      Vector2(-0.3125f, 0.4375f),
      Vector2(0.6875f, -0.5625f),
      Vector2(-0.5625f, 0.6875f),
      Vector2(0.4375f, -0.3125f),
      Vector2(-0.0625f, -0.8125f),
      Vector2(0.9375f, 0.1875f)
    };
    static const uint64_t sobolSequenceSize = sizeof(seq) / sizeof(seq[0]);
    return seq[index % sobolSequenceSize];
}
}
}