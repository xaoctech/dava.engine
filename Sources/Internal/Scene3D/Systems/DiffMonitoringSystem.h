#pragma once

//#define DIFF_MONITORING_ENABLED
#define DIFF_MONITORING_PROFILE

#ifdef DIFF_MONITORING_ENABLED

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedMeta.h"
#include "Entity/SceneSystem.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
struct TypeCode
{
    static uint16 Get(const Type* type)
    {
        uint32 index = GetTypeCodeIndex();
        const Type* dtype = type->Decay();

        void* data = dtype->GetUserData(index);
        uint16 code = *reinterpret_cast<uint16*>(&data);

        DVASSERT(code > 0);
        return code;
    }

    static const Type* Get(uint16 code)
    {
        auto& map = TypeCodeMap();

        auto it = map.find(code);
        if (it != map.end())
        {
            return it->second;
        }

        return nullptr;
    }

    static void Register(const Type* type, uint16 code)
    {
        uint32 index = GetTypeCodeIndex();
        const Type* dtype = type->Decay();

        DVASSERT(code > 0);

        void* data = reinterpret_cast<void*>(code);
        dtype->SetUserData(index, data);

        TypeCodeMap().insert({ code, dtype });
    }

    static uint32 GetTypeCodeIndex()
    {
        static uint32 index = Type::AllocUserData();
        return index;
    }

    static Map<uint16, const Type*>& TypeCodeMap()
    {
        static Map<uint16, const Type*> map;
        return map;
    }
};

class AnyBitIOStream
{
public:
    using SizeT = uint32;

    AnyBitIOStream(void* data, size_t size)
    {
        DVASSERT(size > sizeof(uint32));

        memStart = reinterpret_cast<uint8*>(data);
        memEnd = memStart + static_cast<ptrdiff_t>(size);
        curPtr = nullptr;
        curEnd = nullptr;
    }

    bool Eof()
    {
        return (curPtr >= curEnd);
    }

    Any Read()
    {
        DVASSERT(nullptr != curPtr);
        DVASSERT(!Eof());

        uint16 code = 0;
        uint16 size = 0;

        Rd(&code, sizeof(code));
        Rd(&size, sizeof(size));

        const Type* type = TypeCode::Get(code);
        DVASSERT(nullptr != type);

        // little optimization hack - let Any copy data by itself,
        // we should just increment rdPtr by our self
        DVASSERT(curPtr + static_cast<ptrdiff_t>(size) <= curEnd);
        const void* data = curPtr;
        curPtr += size;

        if (type == Type::Instance<FastName>())
        {
            return Any(FastName(static_cast<const char*>(data), size));
        }
        else
        {
            return Any(data, type);
        }
    }

    size_t Write(const Any& any)
    {
        return Write(any.GetData(), any.GetType());
    }

    size_t Write(const void* data, const Type* type)
    {
        DVASSERT(nullptr != curPtr);
        DVASSERT(!Eof());

        if (!hasWrites)
        {
            SizeT sizeReserved = 0;
            Wr(&sizeReserved, sizeof(sizeReserved));

            hasWrites = true;
        }

        uint16 code = TypeCode::Get(type);
        uint16 size = static_cast<uint16>(type->GetSize());

        DVASSERT(code > 0);

        if (type == Type::Instance<FastName>())
        {
            const FastName* fname = static_cast<const FastName*>(data);
            size = static_cast<uint16>(fname->size());
            data = fname->c_str();
        }

        Wr(&code, sizeof(code));
        Wr(&size, sizeof(size));
        Wr(data, size);

        return size + sizeof(code) + sizeof(size);
    }

    void OpenForRead()
    {
        curPtr = memStart;
        curEnd = memEnd;

        SizeT sizeReady = 0;
        Rd(&sizeReady, sizeof(sizeReady));

        curEnd = curPtr + static_cast<ptrdiff_t>(sizeReady);
    }

    void OpenForWrite()
    {
        forWrite = true;
        hasWrites = false;

        curPtr = memStart;
        curEnd = memEnd;
    }

    void Seek(size_t pos)
    {
        curPtr = memStart + static_cast<ptrdiff_t>(pos);

        if (curPtr > curEnd)
        {
            curPtr = curEnd;
        }
    }

    void* Data()
    {
        return memStart;
    }

    size_t Tell()
    {
        return (curPtr - memStart);
    }

    void Flush()
    {
        size_t curPos = Tell();

        if (forWrite && hasWrites)
        {
            SizeT writtenSize = curPtr - memStart - sizeof(SizeT);

            Seek(0);
            Wr(&writtenSize, sizeof(writtenSize));
        }

        Seek(curPos);
    }

    void Close()
    {
        Flush();

        forWrite = false;
        hasWrites = false;
        curPtr = nullptr;
        curEnd = nullptr;
    }

protected:
    inline void Wr(const void* srcData, size_t size)
    {
        DVASSERT(curPtr < curEnd);
        DVASSERT(curEnd - curPtr >= static_cast<ptrdiff_t>(size));

        ::memcpy(curPtr, srcData, size);
        curPtr += size;
    }

    inline void Rd(void* dstData, size_t size)
    {
        DVASSERT(curPtr < curEnd);
        DVASSERT(curEnd - curPtr >= static_cast<ptrdiff_t>(size));

        ::memcpy(dstData, curPtr, size);
        curPtr += size;
    }

    bool forWrite = false;
    bool hasWrites = false;
    uint8* memStart = nullptr;
    uint8* memEnd = nullptr;

    uint8* curEnd = nullptr;
    uint8* curPtr = nullptr;
};

class DiffMonitoringSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(DiffMonitoringSystem, SceneSystem);

    struct WatchPoint
    {
        WatchPoint() = default;
        WatchPoint(Component* component_, const Reflection& fieldRef_, const M::Replicable& fieldMeta_, const Any& fieldKey_, const Vector<Any>& rootKey_)
            : component(component_)
            , fieldRef(fieldRef_)
            , fieldMeta(fieldMeta_)
            , fieldKey(fieldKey_)
            , rootKey(rootKey_)
        {
            fieldValue = fieldRef.GetValue();
        }

        Component* component = nullptr;
        Reflection fieldRef;
        M::Replicable fieldMeta;
        Vector<Any> rootKey;
        Any fieldKey;
        Any fieldValue;
    };

    struct WatchLine
    {
        using Diff = Set<const WatchPoint*>;

        virtual bool CanWatch(const Reflection& filed) const = 0;
        virtual void AddWatchPoint(WatchPoint&& wp) = 0;
        virtual void RemoveWatchPoint(Component* component) = 0;
        virtual void Process() = 0;

        DiffMonitoringSystem::WatchLine::Diff diff;
    };

    DiffMonitoringSystem(Scene* scene, UDPServer* server_, UDPClient* client_);
    ~DiffMonitoringSystem();

    void Process(float32 timeElapsed) override;
    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void AddWatchLine(std::unique_ptr<WatchLine> watchLine);

private:
    UDPServer* server;
    UDPClient* client;
    Vector<std::unique_ptr<WatchLine>> watchLines;

    void StartWatching(Component* component);
    void StopWatching(Component* component);
    void StartFieldWatching(Component* component, Reflection fieldRef, Any fieldKey, Vector<Any> rootKey, const M::Replicable& fieldMeta);

    void OnReceiveClient(const uint8* data, size_t size, uint8, uint32);

    void SaveSingleDiff(AnyBitIOStream& wrStream, const WatchPoint* wp);
    void LoadSingleDiff(AnyBitIOStream& rdStream);
};

} // namespace DAVA

#endif // DIFF_MONITORING_ENABLED
