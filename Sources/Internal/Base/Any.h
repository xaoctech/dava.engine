#pragma once

#include "Base/Type.h"
#include "Base/TypeInheritance.h"
#include "Base/Exception.h"
#include "Base/Private/AutoStorage.h"

namespace DAVA
{
/** 
    \ingroup Base
    The class Any is a type-safe container for single value of any type.
    Stored value is always copied into internal storage. Implementations is encouraged to 
    avoid dynamic allocations for small objects, but such an optimization may only
    be applied to types for which std::is_nothrow_move_constructible returns true.
    \remark This class cannot be inherited.

    Any can be copied.
    - Internal storage with trivial value will also be copied.
    - Internal storage with complex type will be shared the same way as `std::shared_ptr` do.

    Typical usage:
    ```
    void foo()
    {
        int i = 1;
        const char* s = "Hello world";

        Any a;
        a.Set(i);
        std::count << a.Get<int>(); // prints "1"

        a.Set(s);
        std::count << a.Get<const char*>(); // prints "Hello world"
    }
    ```

    TODO: more description
    ...
*/
class Any final
{
public:
    using AnyStorage = AutoStorage<>;
    using CastOp = Any (*)(const Any&);
    using HashOp = size_t (*)(const Any&);

    template <typename T>
    using NotAny = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, bool>::type;

    Any() = default;
    Any(const Any&) = default;

    Any(Any&& any);
    Any(const void* data, const Type* type);

    template <typename T, typename = NotAny<T>>
    Any(T&& value);

    ~Any() = default;

    /** Swaps this Any value with the given `any`. */
    void Swap(Any& any);

    /** Return `true` if Any is empty or `false` if isn't. */
    bool IsEmpty() const;

    /** Clears Any to its empty state. */
    void Clear();

    /** Gets the type of contained value. `null` will be returned if Any is empty. */
    const Type* GetType() const;

    /** Returns `true` if value with specified type T can get be from Any. */
    template <typename T>
    bool CanGet() const;

    /** Gets value with specified type T from internal storage. 
        If specified T can't be get due to type mismatch `DAVA::Exception` will be raised. */
    template <typename T>
    const T& Get() const noexcept(false);

    /** Gets value with specified type T from internal storage.
        If specified T can't be get due to type mismatch then `defaultValue` will be returned. */
    template <typename T>
    const T& GetSafely(const T& defaultValue) const;

    /** Sets the value. Note that incoming value will be copied|moved into Any depending on lvalue|rvalue. */
    template <typename T, typename = NotAny<T>>
    void Set(T&& value);

    /** Sets the value from other rvalue `any`. Note that incoming value becomes empty. */
    void Set(Any&& any);

    /** Sets the value from other `any`. Note that incoming value will be copied. */
    void Set(const Any& any);

    void SetTrivially(const void* data, const Type* type);

    /** Return `true` if contained value can be casted to specified T. */
    template <typename T>
    bool CanCast() const;

    /** Return `true` if contained value can be casted to specified type. */
    bool CanCast(const Type*) const;

    /** Casts contained value to the value with specified type T. DAVA::Exception will be thrown if 
        contained value can't be casted to the value with specified type T. */
    template <typename T>
    T Cast() const noexcept(false);

    /** Casts contained value to the value with specified type. Empty Any will be returned if
        contained value can't be casted to the value with specified type. */
    Any Cast(const Type*) const;

    /** Try to casts contained value to the value with specified type. 
        If cast isn't available `safeValue` will be returned. */
    template <typename T>
    T CastSafely(const T& safeValue) const;

    /** Try to casts contained value to the value with specified type.
        If cast isn't available `safeValue` will be returned. */
    Any CastSafely(const Type*, Any safeValue) const;

    /** Returns new any with same internal data but other type. 
        WARNING: use this method carefully. */
    Any ReinterpretCast(const Type* type) const;

    /** Loads value into Any from specified memory location with specified Type. 
        Loading can be done only from types that are Type::IsTriviallyCopyable. 
        Returns true if value was successfully loaded. */
    bool LoadData(const void* src, const Type* type);

    /** Stores contained value into specified memory location. Storing can
        be done only for values whose type is Type::IsTriviallyCopyable. 
        Returns true if value was successfully loaded. */
    bool StoreData(void* dst, size_t size) const;

    /** Returns pointer on internal data. If Any is empty internal data is unspecified. 
        WARNING: You mustn't change this data in any way cause it may be shared between
        multiple Any instances. */
    const void* GetData() const;

    /** Returns `true` if hash can be calculated for contained value. */
    bool CanHash() const;

    /** Returns hash for contained value. DAVA::Exception will be thrown if 
        hash can't be calculated for contained value/ */
    size_t Hash() const noexcept(false);

    Any& operator=(Any&&);
    Any& operator=(const Any&) = default;

    bool operator==(const Any&) const;
    bool operator!=(const Any&) const;

private:
    const Type* type = nullptr;
    AnyStorage anyStorage;
};

/** 
    Allow to register Any cast operation for user types.
*/
template <typename From, typename To>
struct AnyCast
{
    static void Register(Any::CastOp op);
    static void RegisterDefault();
};

/**
    Allow to register Any hash operation for user types.
*/
template <typename T>
struct AnyHash
{
    static void Register(Any::HashOp op);
    static void RegisterDefault();
};

std::ostream& operator<<(std::ostream& os, const DAVA::Any& any);

} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::Any>;

} // namespace std

#define __Dava_Any__
#include "Base/Private/Any_impl.h"
