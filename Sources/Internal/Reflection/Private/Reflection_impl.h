#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Reflection_pre_impl.h"
#include "Reflection/Private/Wrappers/ValueWrapperDefault.h"
#include "Reflection/Private/Wrappers/ValueWrapperObject.h"

namespace DAVA
{
namespace ReflectionDetail
{
}

inline bool Reflection::IsReadonly() const
{
    return valueWrapper->IsReadonly(object);
}

inline const Type* Reflection::GetValueType() const
{
    return valueWrapper->GetType(object);
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return valueWrapper->GetValueObject(object);
}

inline ReflectedObject Reflection::GetDirectObject() const
{
    return object;
}

inline Any Reflection::GetValue() const
{
    return valueWrapper->GetValue(object);
}

inline bool Reflection::SetValue(const Any& value) const
{
    return valueWrapper->SetValue(object, value);
}

inline bool Reflection::SetValueWithCast(const Any& value) const
{
    return valueWrapper->SetValueWithCast(object, value);
}

inline bool Reflection::IsValid() const
{
    return (nullptr != valueWrapper && object.IsValid());
}

template <typename T>
inline const T* Reflection::GetMeta() const
{
    return static_cast<const T*>(GetMeta(Type::Instance<T>()));
}

template <typename T>
Reflection Reflection::Create(T* objectPtr, const ReflectedMeta* objectMeta)
{
    if (nullptr != objectPtr)
    {
        static ValueWrapperDefault<T> objectValueWrapper;
        return Reflection(ReflectedObject(objectPtr), &objectValueWrapper, nullptr, objectMeta);
    }

    return Reflection();
}

} // namespace DAVA
