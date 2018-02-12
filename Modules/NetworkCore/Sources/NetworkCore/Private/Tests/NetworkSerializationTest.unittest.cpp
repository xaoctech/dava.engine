#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"
#include "Base/Exception.h"
#include "Base/BaseTypes.h"
#include "Math/Matrix4.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/Private/NetworkSerialization.h"
using namespace DAVA;

DAVA_TESTCLASS (NetworkSerializationTest)
{
    DAVA_TEST (PODSerializationTest)
    {
        uint8 out[100] = {};
        int8 ti8 = -1;
        uint8 tui8 = 2;
        int16 ti16 = -3;
        uint16 tui16 = 4;
        int32 ti32 = -5;
        uint32 tui32 = 6;
        int64 ti64 = -7;
        uint64 tui64 = 8;

        uint32 size = 0;

        size += NetworkSerialization::Save(out + size, ti8);
        size += NetworkSerialization::Save(out + size, tui8);
        size += NetworkSerialization::Save(out + size, ti16, tui16);
        size += NetworkSerialization::Save(out + size, ti32, tui32, ti64, tui64);

        uint8 in[100] = {};
        Memcpy(in, out, size);

        size = 0;
        ti8 = 0;
        tui8 = 0;
        ti16 = 0;
        tui16 = 0;
        ti32 = 0;
        tui32 = 0;
        ti64 = 0;
        tui64 = 0;

        size += NetworkSerialization::Load(in + size, ti8);
        size += NetworkSerialization::Load(in + size, tui8);
        size += NetworkSerialization::Load(in + size, ti16, tui16);
        NetworkSerialization::Load(in + size, ti32, tui32, ti64, tui64);

        TEST_VERIFY(ti8 == -1);
        TEST_VERIFY(tui8 == 2);
        TEST_VERIFY(ti16 == -3);
        TEST_VERIFY(tui16 == 4);
        TEST_VERIFY(ti32 == -5);
        TEST_VERIFY(tui32 == 6);
        TEST_VERIFY(ti64 == -7);
        TEST_VERIFY(tui64 == 8);
    };

    DAVA_TEST (SimpleStructSerializationTest)
    {
        using namespace NetworkSerialization;
        struct SimpleStruct
        {
            int8 a;
            uint32 b;
            uint64 c;
            float32 d;

            SERIALIZABLE(a, b, c, d)
        } packed;

        packed.a = 1;
        packed.b = 2;
        packed.c = 3;
        packed.d = 4;

        const uint32 size = NetworkSerialization::GetSize(packed);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, packed));

        uint8* in = out;
        SimpleStruct unpacked;
        TEST_VERIFY(size == NetworkSerialization::Load(in, unpacked));

        TEST_VERIFY(unpacked.a == 1);
        TEST_VERIFY(unpacked.b == 2);
        TEST_VERIFY(unpacked.c == 3);
        TEST_VERIFY(unpacked.d == 4);
    };

    DAVA_TEST (Matrix4SerializationTest)
    {
        using namespace NetworkSerialization;
        struct StructWithMatrix4
        {
            Matrix4 localTransform;

            SERIALIZABLE(localTransform)
        } structWithMatrix4;

        const uint32 size = NetworkSerialization::GetSize(structWithMatrix4);
        uint8 out[100] = {};
        structWithMatrix4.localTransform._data[0][0] = 14.f;
        TEST_VERIFY(size == NetworkSerialization::Save(out, structWithMatrix4));

        StructWithMatrix4 unpacked;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpacked));

        TEST_VERIFY(unpacked.localTransform._data[0][0] == 14.f);
    }

    DAVA_TEST (MapSerializationTest)
    {
        using MapIntInt = Map<uint64, uint64>;
        MapIntInt pack;
        pack.emplace(1, 10);
        const uint32 size = NetworkSerialization::GetSize(pack);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, pack));
        MapIntInt unpack;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpack));
        TEST_VERIFY(unpack[1] == 10);
    }

    DAVA_TEST (UnorderedSetSerializationTest)
    {
        UnorderedSet<uint32> pack = { 1, 2, 88 };
        const uint32 size = NetworkSerialization::GetSize(pack);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, pack));
        UnorderedSet<uint32> unpack;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpack));
        TEST_VERIFY(unpack.find(88) != unpack.end());
    }

    DAVA_TEST (VectorSerializationTest)
    {
        using VectorInt = Vector<uint8>;
        VectorInt pack(10);
        pack[1] = 10;
        const uint32 size = NetworkSerialization::GetSize(pack);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, pack));
        VectorInt unpack;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpack));
        TEST_VERIFY(unpack[1] == 10);
    }

    DAVA_TEST (SmartPointerSerializationTest)
    {
        using IntPtr = std::unique_ptr<int>;
        IntPtr pack(new int(14));
        const uint32 size = NetworkSerialization::GetSize(pack);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, pack));
        IntPtr unpack;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpack));
        TEST_VERIFY(*unpack == 14);
    }

    DAVA_TEST (EmptySmartPointerSerializationTest)
    {
        using IntPtr = std::unique_ptr<int>;
        IntPtr pack;
        const uint32 size = NetworkSerialization::GetSize(pack);
        uint8 out[100] = {};
        TEST_VERIFY(size == NetworkSerialization::Save(out, pack));
        IntPtr unpack;
        TEST_VERIFY(size == NetworkSerialization::Load(out, unpack));
        TEST_VERIFY(!unpack);
    }

    DAVA_TEST (MapToVectorPtrSerializationTest)
    {
        using SerializedState = Vector<uint8>;
        using SerializedStatePtr = std::unique_ptr<SerializedState>;
        using ComponentToSerializedStatePtr = UnorderedMap<uint32, SerializedStatePtr>;
        ComponentToSerializedStatePtr compToState;
        uint8 out[100];
        uint8 x = 10;
        TEST_VERIFY(NetworkSerialization::GetSize(x) == NetworkSerialization::Save(out, x));

        SerializedState* ss = new SerializedState(20);
        uint32 saveSize = NetworkSerialization::Save(out, *ss);
        uint32 size = NetworkSerialization::GetSize(*ss);
        TEST_VERIFY(saveSize == size);

        SerializedStatePtr ptr(ss);
        TEST_VERIFY(NetworkSerialization::GetSize(ptr) == NetworkSerialization::Save(out, ptr));
        ptr->at(14) = 14;
        compToState.emplace(1, std::move(ptr));
        saveSize = NetworkSerialization::Save(out, compToState);
        size = NetworkSerialization::GetSize(compToState);
        TEST_VERIFY(saveSize == size);

        ComponentToSerializedStatePtr unpacked;
        TEST_VERIFY(NetworkSerialization::Load(out, unpacked) == size);
        TEST_VERIFY(unpacked[1]->at(14) == 14);
    }
};



#endif // !defined(__DAVAENGINE_ANDROID__)
