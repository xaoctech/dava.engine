#include "Base/Any.h"
#include "Base/FastName.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "Debug/Backtrace.h"

namespace DAVA
{
namespace AnyDetails
{
using CastMap = UnorderedMap<const Type*, Any::CastOp>;

uint32_t GetCastOpIndex()
{
    static uint32_t index = Type::AllocUserData();
    return index;
}

uint32_t GetHashOpIndex()
{
    static uint32_t index = Type::AllocUserData();
    return index;
}

Any::CastOp GetCastOp(const Type* from, const Type* to)
{
    uint32_t index = GetCastOpIndex();
    CastMap* castMap = static_cast<CastMap*>(from->GetUserData(index));
    if (nullptr != castMap)
    {
        auto it = castMap->find(to);
        if (it != castMap->end())
        {
            return it->second;
        }
    }

    return nullptr;
}

void SetCastOp(const Type* from, const Type* to, Any::CastOp op)
{
    uint32_t index = GetCastOpIndex();
    CastMap* castMap = static_cast<CastMap*>(from->GetUserData(index));
    if (nullptr == castMap)
    {
        castMap = new CastMap();
        from->SetUserData(index, castMap, [](void* ptr) {
            delete static_cast<CastMap*>(ptr);
        });
    }

    castMap->operator[](to) = op;
}

bool CanCast(const Type* fromType, const Type* toType)
{
    if (fromType == toType)
        return true;

    if (fromType == nullptr || toType == nullptr)
        return false;

    if (fromType->IsPointer() && toType->IsPointer())
    {
        const Type* derefFrom = fromType->Deref();
        const Type* derefTo = toType->Deref();
        return (derefFrom->IsVoid() || derefTo->IsVoid()) ? true : TypeInheritance::CanCast(derefFrom, derefTo);
    }

    if ((fromType->IsIntegral() || fromType->IsEnum()) && (toType->IsIntegral() || toType->IsEnum()))
        return true;

    Any::CastOp op = AnyDetails::GetCastOp(fromType, toType);
    return (nullptr != op);
}

inline Any DoGeneralizedPointerCast(const void* data, const Type* fromType, const Type* toType)
{
    const Type* derefFrom = fromType->Deref();
    const Type* derefTo = toType->Deref();

    if (derefFrom->IsVoid() || derefTo->IsVoid())
    {
        return Any(data, toType);
    }
    else
    {
        void* inPtr = *static_cast<void* const*>(data);
        void* outPtr = nullptr;

        if (TypeInheritance::Cast(derefFrom, derefTo, inPtr, &outPtr))
        {
            return Any(&outPtr, toType);
        }
    }

    return Any();
}

inline Any DoGeneralizedIntegralCast(const void* data, const Type* fromType, const Type* toType)
{
    DVASSERT(fromType == fromType->Decay());
    DVASSERT(toType == toType->Decay());

    uint32_t srcSize = fromType->GetSize();
    uint32_t dstSize = toType->GetSize();

    int64 src = 0;

    if (fromType->IsSigned())
    {
        if (srcSize == 1)
        {
            src = static_cast<int64>(*static_cast<const int8_t*>(data));
        }
        else if (srcSize == 2)
        {
            src = static_cast<int64>(*static_cast<const int16_t*>(data));
        }
        else if (srcSize == 4)
        {
            src = static_cast<int64>(*static_cast<const int32_t*>(data));
        }
        else if (srcSize == 8)
        {
            src = static_cast<int64>(*static_cast<const int64_t*>(data));
        }
    }
    else
    {
        if (srcSize == 1)
        {
            src = static_cast<int64>(*static_cast<const uint8_t*>(data));
        }
        else if (srcSize == 2)
        {
            src = static_cast<int64>(*static_cast<const uint16_t*>(data));
        }
        else if (srcSize == 4)
        {
            src = static_cast<int64>(*static_cast<const uint32_t*>(data));
        }
        else if (srcSize == 8)
        {
            src = static_cast<int64>(*static_cast<const uint64_t*>(data));
        }
    }

    if (toType->IsSigned())
    {
        if (dstSize == 1)
        {
            int8_t dst = static_cast<int8_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 2)
        {
            int16_t dst = static_cast<int16_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 4)
        {
            int32_t dst = static_cast<int32_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 8)
        {
            int64_t dst = static_cast<int64_t>(src);
            return Any(&dst, toType);
        }
    }
    else
    {
        if (dstSize == 1)
        {
            uint8_t dst = static_cast<uint8_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 2)
        {
            uint16_t dst = static_cast<uint16_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 4)
        {
            uint32_t dst = static_cast<uint32_t>(src);
            return Any(&dst, toType);
        }
        else if (dstSize == 8)
        {
            uint64_t dst = static_cast<uint64_t>(src);
            return Any(&dst, toType);
        }
    }

    return Any();
}

Any::HashOp GetHashOp(const Type* type)
{
    Any::HashOp hashOp = nullptr;

    if (type != nullptr)
    {
        void* data = type->GetUserData(GetHashOpIndex());
        if (nullptr != data)
        {
            hashOp = reinterpret_cast<Any::HashOp>(data);
        }
        else if (type->IsPointer())
        {
            static auto pointerHashOp = [](const Any& any) -> size_t {

                const void* data = any.GetData();
                void* pointer = *static_cast<void* const*>(data);

                return std::hash<void*>()(pointer);
            };

            hashOp = pointerHashOp;
        }
        else if (type->IsIntegral() || type->IsEnum())
        {
            static auto integrapHashOp = [](const Any& any) -> size_t {

                const void* data = any.GetData();
                const Type* type = any.GetType();
                uint32_t size = type->GetSize();

                if (type->IsSigned())
                {
                    if (size == 1)
                    {
                        return std::hash<int8_t>()(*static_cast<const int8_t*>(data));
                    }
                    else if (size == 2)
                    {
                        return std::hash<int16_t>()(*static_cast<const int16_t*>(data));
                    }
                    else if (size == 4)
                    {
                        return std::hash<int32_t>()(*static_cast<const int32_t*>(data));
                    }
                    else if (size == 8)
                    {
                        return std::hash<int64_t>()(*static_cast<const int64_t*>(data));
                    }
                }
                else
                {
                    if (size == 1)
                    {
                        return std::hash<uint8_t>()(*static_cast<const uint8_t*>(data));
                    }
                    else if (size == 2)
                    {
                        return std::hash<uint16_t>()(*static_cast<const uint16_t*>(data));
                    }
                    else if (size == 4)
                    {
                        return std::hash<uint32_t>()(*static_cast<const uint32_t*>(data));
                    }
                    else if (size == 8)
                    {
                        return std::hash<uint64_t>()(*static_cast<const uint64_t*>(data));
                    }
                }

                DVASSERT(false); // should never pass here
                return 0;
            };

            hashOp = integrapHashOp;
        }
    }

    return hashOp;
}

} // namespace AnyDetails

bool Any::CanCast(const Type* toType) const
{
    toType = toType->Decay();

    return AnyDetails::CanCast(type, toType);
}

Any Any::Cast(const Type* toType) const
{
    toType = toType->Decay();

    if (type == toType)
        return *this;

    if (type == nullptr || toType == nullptr)
        return Any();

    Any::CastOp op = AnyDetails::GetCastOp(type, toType);
    if (nullptr != op)
    {
        return (*op)(*this);
    }

    if (type->IsPointer() && toType->IsPointer())
    {
        return AnyDetails::DoGeneralizedPointerCast(GetData(), type, toType);
    }

    if ((type->IsIntegral() || type->IsEnum()) && (toType->IsIntegral() || toType->IsEnum()))
    {
        return AnyDetails::DoGeneralizedIntegralCast(GetData(), type, toType);
    }

    return Any();
}

Any Any::CastSafely(const Type* toType, Any safeValue) const
{
    Any ret = Cast(toType);

    if (ret.IsEmpty())
    {
        ret = safeValue;
    }

    return ret;
}

Any Any::ReinterpretCast(const Type* toType) const
{
    Any ret(*this);
    ret.type = toType->Decay();
    return ret;
}

bool Any::LoadData(const void* data, const Type* type_)
{
    type = type_->Decay();

    if (type->IsPointer())
    {
        void** src = reinterpret_cast<void**>(const_cast<void*>(data));
        anyStorage.SetAuto(*src);
        return true;
    }
    else if (type->IsTriviallyCopyable())
    {
        anyStorage.SetData(data, type->GetSize());
        return true;
    }

    return false;
}

bool Any::StoreData(void* data, size_t size) const
{
    if (nullptr != type && size >= type->GetSize())
    {
        if (type->IsPointer())
        {
            void** dst = reinterpret_cast<void**>(data);
            *dst = anyStorage.GetAuto<void*>();
            return true;
        }
        else if (type->IsTriviallyCopyable())
        {
            std::memcpy(data, anyStorage.GetData(), size);
            return true;
        }
    }

    return false;
}

bool Any::operator==(const Any& any) const
{
    if (type == nullptr || any.type == nullptr)
    {
        return type == any.type;
    }

    if (type == any.type)
    {
        Type::CompareOp equalCompareOp = type->GetEqualCompareOp();
        if (nullptr != equalCompareOp)
        {
            return (*equalCompareOp)(GetData(), any.GetData());
        }
    }

    if (type->IsPointer() && any.type->IsPointer())
    {
        return anyStorage.GetSimple<void*>() == any.anyStorage.GetSimple<void*>();
    }

    return false;
}

std::ostream& operator<<(std::ostream& os, const DAVA::Any& any)
{
    const Type* type = any.GetType();

    if (nullptr != type)
    {
        if (type->IsIntegral())
        {
            int64 v = any.Cast<int64>();
            os << v;
        }
        else if (type->IsEnum())
        {
            int64 v = any.Cast<int64>();
            os << "e(" << v << ")";
        }
        else if (type->Is<float32>())
        {
            float32 v = any.Get<float32>();
            os << std::fixed << v;
        }
        else if (type->Is<String>())
        {
            const String& v = any.Get<String>();
            os << "\"" << v << "\"";
        }
        else if (type->Is<FastName>())
        {
            const FastName& v = any.Get<FastName>();
            if (v.IsValid())
            {
                os << "f\"" << v.c_str() << "\"";
            }
            else
            {
                os << "f(!)";
            }
        }
        else if (type->Is<Vector2>())
        {
            const Vector2& v = any.Get<Vector2>();
            os << "v2(" << std::fixed << v.x << ", " << std::fixed << v.y << ")";
        }
        else if (type->Is<Vector3>())
        {
            const Vector3& v = any.Get<Vector3>();
            os << "v3(" << std::fixed << v.x << ", " << std::fixed << v.y << ", " << std::fixed << v.z << ")";
        }
        else if (type->Is<Vector4>())
        {
            const Vector4& v = any.Get<Vector4>();
            os << "v4(" << std::fixed << v.x << ", " << std::fixed << v.y << ", " << std::fixed << v.z << ", " << std::fixed << v.w << ")";
        }
        else if (type->Is<Quaternion>())
        {
            const Quaternion& v = any.Get<Quaternion>();
            os << "q(" << std::fixed << v.x << ", " << std::fixed << v.y << ", " << std::fixed << v.z << ", " << std::fixed << v.w << ")";
        }
        else if (any.CanCast<String>())
        {
            os << "@\"" << any.Cast<String>() << "\"";
        }
        else
        {
            os << "(?) <" << Debug::DemangleFrameSymbol(type->GetName()) << ">";
        }
    }
    else
    {
        os << "(!)";
    }

    return os;
}
} // namespace DAVA
