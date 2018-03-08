#pragma once

namespace DAVA
{
template <typename T>
uint32 ComponentUtils::GetRuntimeId()
{
    return GetRuntimeId(Type::Instance<T>());
}

template <typename... Args>
ComponentMask ComponentUtils::MakeMask()
{
    if (sizeof...(Args) > 0)
    {
        return MakeMask((Type::Instance<Args>())...);
    }
    else
    {
        return ComponentMask();
    }
}

template <typename... Args>
ComponentMask ComponentUtils::MakeMask(const Args*... args)
{
    ComponentMask mask;

    for (const Type* type : { (args)... })
    {
        DVASSERT(type != nullptr);
        mask.Set(type);
    }

    DVASSERT(mask.IsAnySet());
    return mask;
}
} // namespace DAVA
