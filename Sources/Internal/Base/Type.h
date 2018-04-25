#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <bitset>
#include <array>

namespace DAVA
{
namespace TypeDetails
{
template <typename T>
struct TypeHolder;
}

class TypeInheritance;
class Type final
{
    friend class TypeInheritance;

    template <typename T>
    friend struct TypeDetails::TypeHolder;

public:
    struct Seed;

    template <typename T>
    using DecayT = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<std::decay_t<std::remove_pointer_t<T>>>, std::decay_t<T>>;

    template <typename T>
    using DerefT = std::remove_pointer_t<std::decay_t<T>>;

    template <typename T>
    using PointerT = std::add_pointer_t<std::decay_t<T>>;

    using CompareOp = bool (*)(const void*, const void*);
    using SeedCastOp = const Type::Seed* (*)(const void*);

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    const char* GetName() const;

    uint32_t GetSize() const;
    uint32_t GetArrayDimension() const;

    CompareOp GetEqualCompareOp() const;
    SeedCastOp GetSeedCastOp() const;

    const TypeInheritance* GetInheritance() const;

    void SetUserData(uint32_t index, void* data, void (*deleter)(void*) = nullptr) const;
    void* GetUserData(uint32_t index) const;

    template <typename T>
    bool Is() const;

    bool IsVoid() const;
    bool IsConst() const;
    bool IsPointer() const;
    bool IsPointerToConst() const;
    bool IsReference() const;
    bool IsReferenceToConst() const;
    bool IsFundamental() const;
    bool IsTrivial() const;
    bool IsTriviallyCopyable() const;
    bool IsIntegral() const;
    bool IsSigned() const;
    bool IsFloatingPoint() const;
    bool IsEnum() const;
    bool IsAbstract() const;
    bool IsPod() const;
    bool IsArray() const;

    uint32_t GetTypeFlags() const;

    const Type* Decay() const;
    const Type* Deref() const;
    const Type* Pointer() const;
    const Type* GetArrayElementType() const;

    static uint32_t AllocUserData();

    template <typename T>
    static const Type* Instance();

private:
    enum eTypeFlag
    {
        isVoid,
        isConst,
        isPointer,
        isPointerToConst,
        isReference,
        isReferenceToConst,
        isFundamental,
        isTrivial,
        isTriviallyCopyable,
        isIntegral,
        isSigned,
        isFloatingPoint,
        isEnum,
        isAbstract,
        isPod,
        isArray
    };

    struct UserData
    {
        void* data = nullptr;
        void (*deleter)(void*) = nullptr;
    };

    const char* name = nullptr;

    uint16_t size = 0;
    uint16_t arraySize = 0;

    CompareOp equalCompareOp = nullptr;
    SeedCastOp seedCastOp = nullptr;

    Type* const* derefType = nullptr;
    Type* const* decayType = nullptr;
    Type* const* pointerType = nullptr;
    Type* const* arrayElementType = nullptr;

    std::bitset<sizeof(uint32_t) * 8> flags;
    std::unique_ptr<TypeInheritance, void (*)(TypeInheritance*)> inheritance;

    static const size_t userDataStorageSize = 8;
    mutable std::array<UserData, userDataStorageSize> userDataStorage = {};

    Type();
    ~Type();

    TypeInheritance* EditInheritance() const;

    template <typename T>
    static Type* Init();
};

} // namespace DAVA

#define __Dava_Type__
#include "Base/Private/Type_impl.h"
