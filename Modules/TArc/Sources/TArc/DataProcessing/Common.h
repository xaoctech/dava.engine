#pragma once

#include "Base/FastName.h"
#include "Reflection/ReflectedType.h"

namespace DAVA
{
namespace TArc
{
struct FieldDescriptor
{
    const ReflectedType* type = nullptr;
    FastName fieldName;

    FieldDescriptor() = default;

    FieldDescriptor(const ReflectedType* type_, const FastName& fieldName_)
        : type(type_)
        , fieldName(fieldName_)
    {
    }

    bool IsEmpty() const;
};

inline bool FieldDescriptor::IsEmpty() const
{
    return type == nullptr || fieldName.IsValid() == false;
}

} // namespace TArc
} // namespace DAVA