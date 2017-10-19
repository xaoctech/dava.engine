#include "Animation/LinearPropertyAnimation.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
ReflectionAnyPropertyAnimation::ReflectionAnyPropertyAnimation(AnimatedObject* _owner, const Reflection& _ref, float32 _animationTimeLength, Interpolation::FuncType _iFuncType)
    : Animation(_owner, _animationTimeLength, _iFuncType)
    , ref(_ref)
{
}

ReflectionAnyPropertyAnimation::~ReflectionAnyPropertyAnimation() = default;

void ReflectionAnyPropertyAnimation::SetPropertyValue(const Any& value)
{
    ref.SetValueWithCast(value);
}
}