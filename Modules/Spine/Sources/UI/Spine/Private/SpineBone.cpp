#include "UI/Spine/Private/SpineBone.h"

#include <Debug/DVAssert.h>

#include <spine/spine.h>

namespace DAVA
{
SpineBone::SpineBone(spBone* bone)
    : bonePtr(bone)
{
    DVASSERT(bonePtr);
}

Vector2 SpineBone::GetPosition() const
{
    if (bonePtr)
    {
        return Vector2(bonePtr->worldX, -bonePtr->worldY);
    }
    return Vector2();
}

Vector2 SpineBone::GetScale() const
{
    if (bonePtr)
    {
        return Vector2(bonePtr->scaleX, bonePtr->scaleY);
    }
    return Vector2();
}

float32 SpineBone::GetAngle() const
{
    if (bonePtr)
    {
        return float32(-bonePtr->rotation);
    }
    return 0.f;
}
}