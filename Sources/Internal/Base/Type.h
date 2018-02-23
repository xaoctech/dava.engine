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

    using SeedCastOP = const Type::Seed* (*)(const void*);

    Type(Type&&) = delete;
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    uint32_t GetSize() const;
    uint32_t GetArrayDimension() const;

    const char* GetName() const;

    std::type_index GetTypeIndex() const;
    unsigned long GetTypeFlags() const;
    SeedCastOP GetSeedCastOP() const;

    const TypeInheritance* GetInheritance() const;

    void SetUserData(uint32_t index, void* data, void (*deleter)(void*) = nullptr) const;
    void* GetUserData(uint32_t index) const;

    template <typename T>
    bool Is() const;

    bool IsConst() const;
    bool IsPointer() const;
    bool IsReference() const;
    bool IsFundamental() const;
    bool IsTrivial() const;
    bool IsTriviallyCopyable() const;
    bool IsIntegral() const;
    bool IsFloatingPoint() const;
    bool IsEnum() const;
    bool IsAbstract() const;
    bool IsPOD() const;
    bool IsArray() const;

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
        isConst,
        isPointer,
        isPointerToConst,
        isReference,
        isReferenceToConst,
        isFundamental,
        isTrivial,
        isTriviallyCopyable,
        isIntegral,
        isFloatingPoint,
        isEnum,
        isAbstract,
        isPOD,
        isArray
    };

    struct UserData
    {
        using Deleter = void (*)(void*);

        void* data = nullptr;
        Deleter deleter = nullptr;
    };

    uint16_t size = 0;
    uint16_t arraySize = 0;

    const char* name = nullptr;
    const std::type_info* stdTypeInfo = &typeid(void);
    SeedCastOP seedCastOP = nullptr;

    Type* const* derefType = nullptr;
    Type* const* decayType = nullptr;
    Type* const* pointerType = nullptr;
    Type* const* arrayElementType = nullptr;

    std::bitset<sizeof(int) * 8> flags;
    std::unique_ptr<TypeInheritance, void (*)(TypeInheritance*)> inheritance;

    static const size_t userDataStorageSize = 16;
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
