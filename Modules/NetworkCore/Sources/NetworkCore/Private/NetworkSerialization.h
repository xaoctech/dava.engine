#pragma once

#include <Base/BaseTypes.h>
#include <Logger/Logger.h>
#include <Math/Matrix4.h>

namespace DAVA
{
namespace NetworkSerialization
{
template <class T, typename E = void>
class TSerializer
{
public:
    static uint32 Save(uint8* out, const T& object)
    {
        return object.Save(out);
    }

    static uint32 Load(const uint8* in, T& object)
    {
        return object.Load(in);
    }

    static uint32 GetSize(const T& object)
    {
        return object.GetSize();
    }
};

template <class T>
static inline uint32 Save(uint8* out, const T& t);

template <class T>
static inline uint32 Load(const uint8* in, T& t);

template <class T>
static inline uint32 GetSize(const T& t);

template <class A, class B>
class TSerializer<std::pair<A, B>>
{
public:
    static uint32 Save(uint8* out, const std::pair<A, B>& object)
    {
        uint32 size = NetworkSerialization::Save(out, object.first);
        return size + NetworkSerialization::Save(out + size, object.second);
    }

    static uint32 Load(const uint8* in, std::pair<A, B>& object)
    {
        uint32 size = NetworkSerialization::Load(in, object.first);
        return size + NetworkSerialization::Load(in + size, object.second);
    }

    static uint32 GetSize(const std::pair<A, B>& object)
    {
        return NetworkSerialization::GetSize(object.first) + NetworkSerialization::GetSize(object.second);
    }
};

template <class TIter>
class TIterSerializer
{
public:
    static uint32 GetSize(const TIter& object)
    {
        uint32 size = static_cast<uint32>(object.size());
        size = NetworkSerialization::GetSize(size);
        for (const auto& obj : object)
        {
            size += NetworkSerialization::GetSize(obj);
        }

        return size;
    }
};

template <class TVec, class TObj>
class TVectorSerializer : public TIterSerializer<TVec>
{
public:
    static inline uint32 Save(uint8* out, const TVec& object)
    {
        const uint32 size = static_cast<uint32>(object.size());
        uint32 offset = NetworkSerialization::Save(out, size);
        for (const auto& obj : object)
        {
            offset += NetworkSerialization::Save(out + offset, obj);
        }

        return offset;
    }

    static inline uint32 Load(const uint8* in, TVec& object)
    {
        uint32 size;
        NetworkSerialization::Load(in, size);
        uint32 offset = NetworkSerialization::GetSize(size);
        object.clear();
        for (size_t i = 0; i < size; ++i)
        {
            TObj obj;
            offset += NetworkSerialization::Load(in + offset, obj);
            object.push_back(std::move(obj));
        }

        return offset;
    }
};

template <class TVec, class TKey, class TValue>
class TMapSerializer : public TIterSerializer<TVec>
{
public:
    static inline uint32 Save(uint8* out, const TVec& object)
    {
        uint32 size = static_cast<uint32>(object.size());
        uint32 offset = NetworkSerialization::Save(out, size);
        for (const auto& obj : object)
        {
            offset += NetworkSerialization::Save(out + offset, obj);
        }

        return offset;
    }

    static inline uint32 Load(const uint8* in, TVec& object)
    {
        uint32 size;
        NetworkSerialization::Load(in, size);
        uint32 offset = NetworkSerialization::GetSize(size);
        object.clear();
        for (size_t i = 0; i < size; ++i)
        {
            std::pair<TKey, TValue> obj;
            offset += NetworkSerialization::Load(in + offset, obj);
            object.insert(std::move(obj));
        }

        return offset;
    }
};

template <class TSet, class TObj>
class TSetSerializer : public TIterSerializer<TSet>
{
public:
    static inline uint32 Save(uint8* out, const TSet& object)
    {
        uint32 size = static_cast<uint32>(object.size());
        uint32 offset = NetworkSerialization::Save(out, size);
        for (const auto& obj : object)
        {
            offset += NetworkSerialization::Save(out + offset, obj);
        }

        return offset;
    }

    static inline uint32 Load(const uint8* in, TSet& object)
    {
        uint32 size;
        uint32 offset = NetworkSerialization::Load(in, size);
        object.clear();
        for (size_t i = 0; i < size; ++i)
        {
            TObj obj;
            offset += NetworkSerialization::Load(in + offset, obj);
            object.insert(std::move(obj));
        }

        return offset;
    }
};

template <class T>
class TPodSerializer
{
public:
    static inline uint32 Save(uint8* out, const T& object)
    {
        const uint32 size = GetSize(object);
        Memcpy(out, (uint8*)&object, size);
        return size;
    }

    static inline uint32 Load(const uint8* in, T& object)
    {
        const uint32 size = GetSize(object);
        Memcpy((uint8*)&object, in, size);
        return size;
    }

    static uint32 GetSize(const T& object)
    {
        return sizeof(T);
    }
};

template <class TPtr, class T>
class TSmartPtrSerializer
{
public:
    static inline uint32 Save(uint8* out, const TPtr& objectPtr)
    {
        const uint8 hasObject = (nullptr != objectPtr);
        uint32 offset = NetworkSerialization::Save(out, hasObject);
        if (hasObject)
        {
            offset += NetworkSerialization::Save(out + offset, *objectPtr);
        }

        return offset;
    }

    static inline uint32 Load(const uint8* in, TPtr& objectPtr)
    {
        uint8 hasObject = 0;
        uint32 offset = NetworkSerialization::Load(in, hasObject);
        if (hasObject)
        {
            objectPtr.reset(new T());
            offset += NetworkSerialization::Load(in + offset, *objectPtr);
        }

        return offset;
    }

    static uint32 GetSize(const TPtr& objectPtr)
    {
        uint32 size = sizeof(uint8);
        if (objectPtr)
        {
            size += NetworkSerialization::GetSize(*objectPtr);
        }

        return size;
    }
};

template <class T>
class TSerializer<Vector<T>>

: public TVectorSerializer<Vector<T>, T>
{
};

template <class T>
class TSerializer<List<T>> : public TVectorSerializer<List<T>, T>
{
};

template <>
class TSerializer<String> : public TVectorSerializer<String, char>
{
};

template <class K, class V>
class TSerializer<Map<K, V>>

: public TMapSerializer<Map<K, V>, K, V>
{
};

template <class K, class V>
class TSerializer<UnorderedMap<K, V>>

: public TMapSerializer<UnorderedMap<K, V>, K, V>
{
};

template <class T>
class TSerializer<Set<T>>

: public TSetSerializer<Set<T>, T>
{
};

template <class T>
class TSerializer<UnorderedSet<T>>
: public TSetSerializer<UnorderedSet<T>, T>
{
};

template <class T>
class TSerializer<std::unique_ptr<T>> : public TSmartPtrSerializer<std::unique_ptr<T>, T>
{
};

template <class T>
class TSerializer<std::shared_ptr<T>> : public TSmartPtrSerializer<std::shared_ptr<T>, T>
{
};

template <>
class TSerializer<Matrix4> : public TPodSerializer<Matrix4>
{
};

template <>
class TSerializer<Vector3> : public TPodSerializer<Vector3>
{
};

template <>
class TSerializer<Quaternion> : public TPodSerializer<Quaternion>
{
};

template <class T>
class TSerializer<T, typename std::enable_if_t<std::is_pod<T>::value>> : public TPodSerializer<T>
{
};

template <class T>
static inline uint32 Save(uint8* out, const T& t)
{
    return TSerializer<T>::Save(out, t);
}

template <class T, class... Args>
static inline uint32 Save(uint8* out, const T& first, Args&... args)
{
    const uint32 size = NetworkSerialization::Save<T>(out, first);
    return size + NetworkSerialization::Save(out + size, args...);
}

template <class T>
static inline uint32 Load(const uint8* in, T& t)
{
    return TSerializer<T>::Load(in, t);
}

template <class T, class... Args>
static inline uint32 Load(const uint8* in, T& first, Args&... args)
{
    const uint32 size = NetworkSerialization::Load<T>(in, first);
    return size + NetworkSerialization::Load(in + size, args...);
}

template <class T>
static inline uint32 GetSize(const T& t)
{
    return TSerializer<T>::GetSize(t);
}

template <class T, class... Args>
static inline uint32 GetSize(const T& first, Args&... args)
{
    return NetworkSerialization::GetSize<T>(first) + NetworkSerialization::GetSize(args...);
}

} // namespace NetworkSerialization
} //namespace DAVA

#define SERIALIZABLE(...)                                                                                           \
    inline DAVA::uint32 Save(DAVA::uint8* out) const { return DAVA::NetworkSerialization::Save(out, __VA_ARGS__); } \
    inline DAVA::uint32 Load(const DAVA::uint8* in)  { return DAVA::NetworkSerialization::Load(in, __VA_ARGS__);  } \
    inline DAVA::uint32 GetSize() const              { return DAVA::NetworkSerialization::GetSize(__VA_ARGS__);   }