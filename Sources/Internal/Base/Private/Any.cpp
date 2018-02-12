#include "Base/Any.h"
#include "Base/FastName.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"

namespace DAVA
{
Any Any::ReinterpretCast(const Type* type) const
{
    Any ret(*this);
    ret.type = type;
    return ret;
}

bool Any::LoadData(const void* data, const Type* type_)
{
    type = type_;

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
    if (any.type == nullptr || type == nullptr)
    {
        return (any.type == type);
    }

    if (any.type->IsPointer() && type->IsPointer())
    {
        return anyStorage.GetSimple<void*>() == any.anyStorage.GetSimple<void*>();
    }

    if (type == any.type)
    {
        if (type->IsTriviallyCopyable())
        {
            return (0 == std::memcmp(anyStorage.GetData(), any.anyStorage.GetData(), type->GetSize()));
        }

        DVASSERT(nullptr != compareFn);
        return (*compareFn)(*this, any);
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
            int v = any.Cast<int>();
            os << "Enum [" << v << "]";
        }
        else if (type->Is<float32>())
        {
            float32 v = any.Get<float32>();
            os << std::fixed << v;
        }
        else if (type->Is<String>())
        {
            const String& v = any.Get<String>();
            os << "'" << v << "'";
        }
        else if (type->Is<FastName>())
        {
            const FastName& v = any.Get<FastName>();
            os << "'" << v.c_str() << "'";
        }
        else if (type->Is<Vector2>())
        {
            const Vector2& v = any.Get<Vector2>();
            os << "Vec2 [" << std::fixed << v.x << "," << std::fixed << v.y << "]";
        }
        else if (type->Is<Vector3>())
        {
            const Vector3& v = any.Get<Vector3>();
            os << "Vec3 [" << std::fixed << v.x << "," << std::fixed << v.y << "," << std::fixed << v.z << "]";
        }
        else if (type->Is<Vector4>())
        {
            const Vector4& v = any.Get<Vector4>();
            os << "Vec4 [" << std::fixed << v.x << "," << std::fixed << v.y << "," << std::fixed << v.z << "," << std::fixed << v.w << "]";
        }
        else if (type->Is<Quaternion>())
        {
            const Quaternion& v = any.Get<Quaternion>();
            os << "Qua [" << std::fixed << v.x << "," << std::fixed << v.y << "," << std::fixed << v.z << "," << std::fixed << v.w << "]";
        }
        else if (type)
        {
            os << "??? " << type->GetName();
        }
    }
    else
    {
        os << "[empty]";
    }

    return os;
}

} // namespace DAVA
