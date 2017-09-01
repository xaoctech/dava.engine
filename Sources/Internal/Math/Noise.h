#pragma once

#include <Base/BaseTypes.h>
#include <Math/Vector.h>

namespace DAVA
{
float32 PerlinNoise3d(const Vector3& position, float32 wrap);
Vector3 Generate4OctavesPerlin(const Vector3& p);
}