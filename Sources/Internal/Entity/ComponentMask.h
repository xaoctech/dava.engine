#pragma once

#include <Base/Bitset.h>
#include <Base/Type.h>

namespace DAVA
{
struct ComponentMask
{
    using Bits = Bitset<128>;

    ComponentMask() = default;
    ~ComponentMask() = default;

    explicit ComponentMask(unsigned long long val);

    void Reset();

    template <typename T>
    void Set(bool value = true);

    void Set(const Type* type, bool value = true);

    template <typename T>
    bool IsSet() const;

    bool IsSet(const Type* type) const;

    bool IsAnySet() const;

    bool operator==(const ComponentMask& other) const;
    bool operator!=(const ComponentMask& other) const;

    ComponentMask operator~() const;
    ComponentMask& operator&=(const ComponentMask& other);
    ComponentMask& operator|=(const ComponentMask& other);
    ComponentMask& operator^=(const ComponentMask& other);

    friend ComponentMask operator&(const ComponentMask& lf, const ComponentMask& rh);
    friend ComponentMask operator|(const ComponentMask& lf, const ComponentMask& rh);
    friend ComponentMask operator^(const ComponentMask& lf, const ComponentMask& rh);

    Bits bits;

private:
    ComponentMask(const Bits& bits);
};

inline ComponentMask::ComponentMask(const ComponentMask::Bits& bits)
    : bits(bits)
{
}

inline ComponentMask::ComponentMask(unsigned long long val)
    : bits(val)
{
}

inline void ComponentMask::Reset()
{
    bits.reset();
}

template <typename T>
inline void ComponentMask::Set(bool value)
{
    Set(Type::Instance<T>(), value);
}

template <typename T>
inline bool ComponentMask::IsSet() const
{
    return IsSet(Type::Instance<T>());
}

inline bool ComponentMask::IsAnySet() const
{
    return bits.any();
}

inline bool ComponentMask::operator==(const ComponentMask& other) const
{
    return bits == other.bits;
}

inline bool ComponentMask::operator!=(const ComponentMask& other) const
{
    return bits != other.bits;
}

inline ComponentMask ComponentMask::operator~() const
{
    return ComponentMask(~bits);
}

inline ComponentMask& ComponentMask::operator&=(const ComponentMask& other)
{
    bits &= other.bits;
    return *this;
}

inline ComponentMask& ComponentMask::operator|=(const ComponentMask& other)
{
    bits |= other.bits;
    return *this;
}

inline ComponentMask& ComponentMask::operator^=(const ComponentMask& other)
{
    bits ^= other.bits;
    return *this;
}

inline ComponentMask operator&(const ComponentMask& lf, const ComponentMask& rh)
{
    return ComponentMask(lf.bits & rh.bits);
}

inline ComponentMask operator|(const ComponentMask& lf, const ComponentMask& rh)
{
    return ComponentMask(lf.bits | rh.bits);
}

inline ComponentMask operator^(const ComponentMask& lf, const ComponentMask& rh)
{
    return ComponentMask(lf.bits ^ rh.bits);
}

} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::ComponentMask>
{
    size_t operator()(const DAVA::ComponentMask& mask) const
    {
        return std::hash<decltype(mask.bits)>{}(mask.bits);
    }
};
} // namespace std
