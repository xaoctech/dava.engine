#pragma once

#include <Base/BaseTypes.h>
#include <Math/Vector.h>

struct spBone;

namespace DAVA
{
class SpineBone
{
public:
    SpineBone(spBone* bone);

    Vector2 GetPosition() const;
    Vector2 GetScale() const;
    float32 GetAngle() const;

private:
    spBone* bonePtr = nullptr;
};
}