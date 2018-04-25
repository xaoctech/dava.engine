#pragma once
#ifndef __Dava_Any__
#include "Base/Any.h"
#endif

namespace DAVA
{
namespace AnyDetails
{
using CanGetDefault = std::integral_constant<int, 0>;
using CanGetPointer = std::integral_constant<int, 1>;
using CanGetVoidPointer = std::integral_constant<int, 2>;

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetDefault)
{
    const Type* rt = Type::Instance<T>();
    return (t == rt);
}

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetPointer)
{
    // We should allow cast from "T*" into "const T*".
    // For any type, that is "const T*" type->Decay() will return "T*".
    const Type* rt = Type::Instance<T>();
    return (t == rt || t == rt->Decay());
}

template <typename T>
inline bool CanGetImpl(const Type* t, CanGetVoidPointer)
{
    // any pointer can be get as "void *"
    return t->IsPointer();
}

extern uint32_t GetCastOpIndex();
extern uint32_t GetHashOpIndex();
extern Any::HashOp GetHashOp(const Type* type);
extern Any::CastOp GetCastOp(const Type* from, const Type* to);
extern void SetCastOp(const Type* from, const Type* to, Any::CastOp op);
extern bool CanCast(const Type* from, const Type* to);

} // AnyDetails

inline Any::Any(Any&& any)
{
    Set(std::move(any));
}

template <typename T, typename /* NotAny */>
inline Any::Any(T&& value)
{
    Set(std::forward<T>(value));
}

inline Any::Any(const void* data, const Type* type)
{
    SetTrivially(data, type);
}

inline bool Any::IsEmpty() const
{
    return (nullptr == type);
}

inline void Any::Clear()
{
    anyStorage.Clear();
    type = nullptr;
}

inline const Type* Any::GetType() const
{
    return type;
}

inline Any& Any::operator=(Any&& any)
{
    if (this != &any)
    {
        Set(std::move(any));
    }

    return *this;
}

inline bool Any::operator!=(const Any& any) const
{
    return !operator==(any);
}

template <typename T>
bool Any::CanGet() const
{
    using U = AnyStorage::StorableType<T>;
    using CanGetPointerPolicy = typename std::conditional<std::is_void<std::remove_pointer_t<T>>::value, AnyDetails::CanGetVoidPointer, AnyDetails::CanGetPointer>::type;
    using CanGetPolicy = typename std::conditional<!std::is_pointer<U>::value, AnyDetails::CanGetDefault, CanGetPointerPolicy>::type;

    if (!IsEmpty())
    {
        return AnyDetails::CanGetImpl<U>(type, CanGetPolicy());
    }

    return false;
}

template <typename T>
const T& Any::Get() const
{
    if (CanGet<T>())
        return anyStorage.GetAuto<T>();

    DAVA_THROW(Exception, "Any:: can't be get as requested T");
}

template <typename T>
inline const T& Any::GetSafely(const T& safeValue) const
{
    return CanGet<T>() ? anyStorage.GetAuto<T>() : safeValue;
}

template <>
inline bool Any::CanGet<Any>() const
{
    return true;
}

template <>
inline bool Any::CanGet<const Any&>() const
{
    return true;
}

template <>
inline const Any& Any::Get<Any>() const
{
    return *this;
}

template <>
inline const Any& Any::Get<const Any&>() const
{
    return *this;
}

template <>
inline const Any& Any::GetSafely<Any>(const Any&) const
{
    return *this;
}

template <>
inline const Any& Any::GetSafely<const Any&>(const Any&) const
{
    return *this;
}

inline const void* Any::GetData() const
{
    return anyStorage.GetData();
}

inline void Any::Swap(Any& any)
{
    std::swap(type, any.type);
    std::swap(anyStorage, any.anyStorage);
}

inline void Any::Set(const Any& any)
{
    anyStorage = any.anyStorage;
    type = any.type;
}

inline void Any::Set(Any&& any)
{
    type = any.type;
    anyStorage = std::move(any.anyStorage);

    any.type = nullptr;
}

template <typename T, typename /* NotAny */>
void Any::Set(T&& value)
{
    using U = AnyStorage::StorableType<T>;

    type = Type::Instance<U>();
    anyStorage.SetAuto(std::forward<T>(value));
}

inline void Any::SetTrivially(const void* data, const Type* type_)
{
    DVASSERT(type_->IsTriviallyCopyable());

    type = type_;
    anyStorage.SetData(data, type->GetSize());
}

template <typename T>
inline bool Any::CanCast() const
{
    return CanCast(Type::Instance<T>());
}

template <typename T>
inline T Any::Cast() const
{
    Any ret = Cast(Type::Instance<T>());
    return ret.Get<T>();
}

template <typename T>
inline T Any::CastSafely(const T& safeValue) const
{
    Any ret = Cast(Type::Instance<T>());
    if (!ret.IsEmpty())
    {
        return ret.Get<T>();
    }
    else
    {
        return safeValue;
    }
}

template <>
inline bool Any::CanCast<const Any&>() const
{
    return true;
}

template <>
inline bool Any::CanCast<Any>() const
{
    return true;
}

template <>
inline Any Any::Cast<Any>() const
{
    return *this;
}

template <>
inline const Any& Any::Cast<const Any&>() const
{
    return *this;
}

template <>
inline Any Any::CastSafely<Any>(const Any&) const
{
    return *this;
}

template <>
inline const Any& Any::CastSafely<const Any&>(const Any&) const
{
    return *this;
}

inline bool Any::CanHash() const
{
    HashOp hashOp = AnyDetails::GetHashOp(type);
    return (nullptr != hashOp);
}

inline size_t Any::Hash() const
{
    HashOp hashOp = AnyDetails::GetHashOp(type);

    if (nullptr != hashOp)
        return (*hashOp)(*this);

    DAVA_THROW(Exception, "Any:: there is no hash for such type");
}

template <typename From, typename To>
void AnyCast<From, To>::Register(Any::CastOp op)
{
    AnyDetails::SetCastOp(Type::Instance<From>(), Type::Instance<To>(), op);
}

template <typename From, typename To>
void AnyCast<From, To>::RegisterDefault()
{
    AnyDetails::SetCastOp(Type::Instance<From>(), Type::Instance<To>(), [](const Any& any) -> Any {
        return static_cast<To>(any.Get<From>());
    });
}

template <typename T>
void AnyHash<T>::Register(Any::HashOp op)
{
    static_assert(sizeof(op) <= sizeof(void*), "HashOp should fit sizeof(void*)");
    Type::Instance<T>()->SetUserData(AnyDetails::GetHashOpIndex(), reinterpret_cast<void*>(op));
}

template <typename T>
void AnyHash<T>::RegisterDefault()
{
    Register([](const Any& any) -> size_t {
        std::hash<T> hashFn;
        return hashFn(any.Get<T>());
    });
}

} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::Any>
{
    size_t operator()(const DAVA::Any& v) const
    {
        return v.Hash();
    }
};

} // namespace std
