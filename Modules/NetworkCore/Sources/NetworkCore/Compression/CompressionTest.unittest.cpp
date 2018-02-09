#include <UnitTests/UnitTests.h>

#include <Base/Any.h>
#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Base/FastName.h>
#include <Base/FixedVector.h>
#include <Base/String.h>
#include <Base/TemplateHelpers.h>
#include <Math/Matrix4.h>
#include <Math/Quaternion.h>
#include <Math/Vector.h>

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedTypeDB.h>

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/CompressorRegistrar.h"

using namespace DAVA;

DAVA_TESTCLASS (CompressionTest)
{
    DAVA_TEST (TestAllTypes)
    {
        struct TestItem
        {
            Any any1;
            Any any2;
            Any resultDelta1;
            Any resultDelta2;
            Any resultFull;
            float32 precision;
        };

        const float32 defPrecision = 1e-5f;

        const Matrix4 matrix1{
            1.f, 0.f, 0.123f, 0.f,
            0.f, -0.435f, 0.1f, 0.34556f,
            0.f, 0.67897f, 0.5f, -0.2345f,
            0.f, 0.1234f, -0.5f, -1.f
        };
        const Matrix4 matrix2{
            0.1f, 0.f, 0.123f, 0.f,
            1.f, 0.435f, 0.1f, -0.00001f,
            0.f, -0.67897f, 0.1223f, -0.666f,
            0.f, 0.789f, -0.789f, 1.f
        };

        const FixedVector<int> vector1(10, { 1, 2, 3, 4, 5 });
        const FixedVector<int> vector2(10, { 10, 20, 30 });

        const std::array<int, 5> array1 = { 1, 2, 3, 4, 5 };
        const std::array<int, 5> array2 = { 1, 12, 31, -4, 5 };

        //vector1 == vector2;
        enum SimpleEnum
        {
            Simple1 = -100,
            Simple2
        };
        enum class ClassEnum : int
        {
            Class1 = 1,
            Class2
        };

        RegisterEnumCompressor<SimpleEnum>();
        RegisterEnumCompressor<ClassEnum>();

        // clang-format off
        TestItem items[] = {
            { Any{ int8   {10         } }, Any{ int8   {20         } }, Any{ int8   {} }, Any{ int8   {} }, Any{ int8   {} }, 0.f },
            { Any{ uint8  {10         } }, Any{ uint8  {20         } }, Any{ uint8  {} }, Any{ uint8  {} }, Any{ uint8  {} }, 0.f },
            { Any{ int16  {510        } }, Any{ int16  {600        } }, Any{ int16  {} }, Any{ int16  {} }, Any{ int16  {} }, 0.f },
            { Any{ uint16 {510        } }, Any{ uint16 {600        } }, Any{ uint16 {} }, Any{ uint16 {} }, Any{ uint16 {} }, 0.f },
            { Any{ int32  {70250      } }, Any{ int32  {75001      } }, Any{ int32  {} }, Any{ int32  {} }, Any{ int32  {} }, 0.f },
            { Any{ uint32 {70250      } }, Any{ uint32 {75001      } }, Any{ uint32 {} }, Any{ uint32 {} }, Any{ uint32 {} }, 0.f },
            { Any{ int64  {10737422826} }, Any{ int64  {10737437720} }, Any{ int64  {} }, Any{ int64  {} }, Any{ int64  {} }, 0.f },
            { Any{ uint64 {10737422826} }, Any{ uint64 {10737437720} }, Any{ uint64 {} }, Any{ uint64 {} }, Any{ uint64 {} }, 0.f },
            { Any{ float32{14.567f    } }, Any{ float32{54.12f     } }, Any{ float32{} }, Any{ float32{} }, Any{ float32{} }, defPrecision },

            { Any{ Simple1           }, Any{ Simple2           }, Any{ SimpleEnum {} }, Any{ SimpleEnum{} }, Any{ SimpleEnum{} }, 0.f },
            { Any{ ClassEnum::Class1 }, Any{ ClassEnum::Class2 }, Any{ ClassEnum{} }, Any{ ClassEnum{} }, Any{ ClassEnum{} }, 0.f },

            { Any{ String  {"1234567890abcdefghijk"} }, Any{ String  {"qwertyuiopzxcvbnm,.;"} }, Any{ String  {} }, Any{ String  {} }, Any{ String  {} }, 0.f },
            { Any{ FastName{"1234567890abcdefghijk"} }, Any{ FastName{"qwertyuiopzxcvbnm,.;"} }, Any{ FastName{} }, Any{ FastName{} }, Any{ FastName{} }, 0.f },

            { Any{ matrix1 }, Any{ matrix2 }, Any{ Matrix4{} }, Any{ Matrix4{} }, Any{ Matrix4{} }, defPrecision },

            { Any{ Quaternion{0.7071067f, 0.f, 0.f, 0.7071067f} }, Any{ Quaternion{0.f, 0.f, 0.f, 1.f} }, Any{ Quaternion{} }, Any{ Quaternion{} }, Any{ Quaternion{} }, defPrecision },

            { Any{ Vector2{130.44f, 122.4f   } }, Any{ Vector2{-45.6f, 22.3f} }, Any{ Vector2{} }, Any{ Vector2{} }, Any{ Vector2{} }, defPrecision },
            { Any{ Vector3{12.4f, -4.1f, 3.3f} }, Any{ Vector3{1.f, 1.f, 1.f} }, Any{ Vector3{} }, Any{ Vector3{} }, Any{ Vector3{} }, defPrecision },

            { Any{ vector1 }, Any{ vector2 }, Any{ FixedVector<int>{10} }, Any{ FixedVector<int>{10} }, Any{ FixedVector<int>{10} }, 0.f },

            { Any{ array1 }, Any{ array2 }, Any{ std::array<int, 5>{10} }, Any{ std::array<int, 5>{10} }, Any{ std::array<int, 5>{10} }, 0.f },

            { Any{ NetworkID(1) }, Any{ NetworkID(2) }, Any{ NetworkID() }, Any{ NetworkID() }, Any{ NetworkID() }, 0.f },

            { Any{ ComponentMask(1) }, Any{ ComponentMask(3) }, Any{ ComponentMask() }, Any{ ComponentMask() }, Any{ ComponentMask() }, 0.f },
            { Any{ ComponentMask(~0) }, Any{ ComponentMask(0) }, Any{ ComponentMask() }, Any{ ComponentMask() }, Any{ ComponentMask() }, 0.f },
        };
        // clang-format on

        char buffer[4096];
        BitWriter writer(buffer, COUNT_OF(buffer));
        for (size_t i = 0; i < COUNT_OF(items); ++i)
        {
            TestItem& cur = items[i];
            const Type* type1 = cur.any1.GetType();
            const Type* type2 = cur.any2.GetType();
            TEST_VERIFY(type1 == type2);

            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(cur.any1);
            TEST_VERIFY(compressor != nullptr && compressor == CompressionUtils::GetTypeCompressor(type2));
            if (compressor != nullptr)
            {
                bool written = compressor->CompressDelta(cur.any1, cur.any2, 0, cur.precision, writer);
                TEST_VERIFY(written == true);
                written = compressor->CompressDelta(cur.any2, cur.any1, 0, cur.precision, writer);
                TEST_VERIFY(written == true);
                compressor->CompressFull(cur.any2, 0, cur.precision, writer);
            }
        }
        writer.WriteAlignmentBits();
        writer.Flush();
        TEST_VERIFY(writer.IsOverflowed() == false);

        BitReader reader(buffer, writer.GetBytesWritten());
        for (size_t i = 0; i < COUNT_OF(items); ++i)
        {
            TestItem& cur = items[i];
            const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(cur.any1);
            TEST_VERIFY(compressor != nullptr);
            if (compressor != nullptr)
            {
                compressor->DecompressDelta(cur.any1, cur.resultDelta1, 0, reader);
                compressor->DecompressDelta(cur.any2, cur.resultDelta2, 0, reader);
                compressor->DecompressFull(cur.resultFull, 0, reader);

                const Type* typeAny1 = cur.any1.GetType();
                const Type* typeAny2 = cur.any2.GetType();
                const Type* typeDelta1 = cur.resultDelta1.GetType();
                const Type* typeDelta2 = cur.resultDelta2.GetType();
                const Type* typeFull = cur.resultFull.GetType();

                TEST_VERIFY(typeAny1 == typeAny2);
                TEST_VERIFY(typeAny1 == typeDelta1);
                TEST_VERIFY(typeAny1 == typeDelta2);
                TEST_VERIFY(typeAny1 == typeFull);

                TEST_VERIFY(compressor->IsEqual(cur.resultDelta1, cur.any2, cur.precision));
                TEST_VERIFY(compressor->IsEqual(cur.resultDelta2, cur.any1, cur.precision));
                TEST_VERIFY(compressor->IsEqual(cur.resultFull, cur.any2, cur.precision));
            }
        }
        reader.ReadAlignmentBits();
        TEST_VERIFY(reader.IsOverflowed() == false);
        TEST_VERIFY(writer.GetBitsWritten() == reader.GetBitsRead());
    }

    template <typename T>
    struct CompressTestItem
    {
        T v1;
        T v2;
    };

    DAVA_TEST (FloatQuantizeTest)
    {
        {
            const float32 precision = 0.0001f;
            M::FloatQuantizeParam qparam(1.f, precision);
            const CompressTestItem<float32> testItems[] = {
                { 0.f, 1.f },
                { 0.f, 0.5f },
                { 0.f, -1.f },
                { 0.f, 0.5f },
                { 0.0005f, 0.005f },
                { 0.00005f, 1.01234f },
                { 0.123456f, 0.987654f },
                { -0.123456f, -0.987654f },
                { 0.333333f, 0.777777f },
            };
            FloatQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
        {
            const float32 precision = 0.01f;
            M::FloatQuantizeParam qparam(10000.f, precision);
            const CompressTestItem<float32> testItems[] = {
                { -10000.234f, 10000.5678f },
                { 0.f, -10000.5f },
                { 0.f, 10000.5f },
                { 2480.5675f, 0.5f },
                { 0.123456f, 0.987654f },
                { -0.123456f, -0.987654f },
                { 0.333333f, 0.777777f },
            };
            FloatQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
    }

    void FloatQuantizer(int tag, const CompressTestItem<float32>* items, uint32 n, const M::FloatQuantizeParam* qparam)
    {
        uint8 buf[1000];
        CompressionScheme scheme = CompressionUtils::MakeCompressionScheme(qparam);
        float32 precision = qparam->precision;
        const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(Type::Instance<float32>());

        BitWriter w(buf, 1000);
        for (uint32 i = 0; i < n; ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            compressor->CompressDelta(a1, a2, scheme, precision, w);
            compressor->CompressDelta(a2, a1, scheme, precision, w);
            compressor->CompressFull(a1, scheme, precision, w);
            compressor->CompressFull(a2, scheme, precision, w);
        }
        w.WriteAlignmentBits();
        w.Flush();

        BitReader r(buf, w.GetBytesWritten());
        for (uint32 i = 0; i < n && !r.IsOverflowed(); ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            Any ar{ 0.f };

            float32 v1 = items[i].v1;
            float32 v2 = items[i].v2;
            float32 vq;
            float32 diff;

            compressor->DecompressDelta(a1, ar, scheme, r);
            vq = ar.Get<float32>();
            diff = std::abs(vq - v2);
            TEST_VERIFY_WITH_MESSAGE(diff < precision, Format("tag=%d n=%u original=%f quantized=%f diff=%f", tag, i, v2, vq, diff));

            compressor->DecompressDelta(a2, ar, scheme, r);
            vq = ar.Get<float32>();
            diff = std::abs(vq - v1);
            TEST_VERIFY_WITH_MESSAGE(diff < precision, Format("tag=%d n=%u original=%f quantized=%f diff=%f", tag, i, v1, vq, diff));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<float32>();
            diff = std::abs(vq - v1);
            TEST_VERIFY_WITH_MESSAGE(diff < precision, Format("tag=%d n=%u original=%f quantized=%f diff=%f", tag, i, v1, vq, diff));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<float32>();
            diff = std::abs(vq - v2);
            TEST_VERIFY_WITH_MESSAGE(diff < precision, Format("tag=%d n=%u original=%f quantized=%f diff=%f", tag, i, v2, vq, diff));
        }
        TEST_VERIFY(!r.IsOverflowed());
    }

    DAVA_TEST (IntCompressorTest)
    {
        {
            M::IntCompressParam cparam(10, 5);
            const CompressTestItem<int16> testItems[] = {
                { 0, 1 },
                { -10, -6 },
                { -2, 2 },
                { 0, 5 },
                { -7, -1 },
                { 1, 7 },
            };
            IntCompressor(__LINE__, testItems, COUNT_OF(testItems), &cparam);
        }
        {
            M::IntCompressParam cparam(1000, 1000);
            const CompressTestItem<uint32> testItems[] = {
                { 0, 1 },
                { 0, 1000 },
                { 255, 648 },
            };
            IntCompressor(__LINE__, testItems, COUNT_OF(testItems), &cparam);
        }
        {
            M::Int64CompressParam cparam(1000, 1000);
            const CompressTestItem<int64> testItems[] = {
                { 0, 1 },
                { 0, 1000 },
                { 255, 648 },
            };
            IntCompressor(__LINE__, testItems, COUNT_OF(testItems), &cparam);
        }
        {
            M::Int64CompressParam cparam(100'000'000'000, 100'000'000'000);
            const CompressTestItem<int64> testItems[] = {
                { 0, 1 },
                { 0, 60'010'000'123 },
                { 0, -60'010'000'123 },
                { -30'000'000'000, 10'034'050'060 },
                { -30'000'000'000, -345 },
                { 30'000'000'000, 345 },
            };
            IntCompressor(__LINE__, testItems, COUNT_OF(testItems), &cparam);
        }
    }

    template <typename T, typename P>
    void IntCompressor(int tag, const CompressTestItem<T>* items, uint32 n, const P* cparam)
    {
        uint8 buf[1000];
        CompressionScheme scheme = CompressionUtils::MakeCompressionScheme(cparam);
        const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(Type::Instance<T>());

        BitWriter w(buf, 1000);
        for (uint32 i = 0; i < n; ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            compressor->CompressDelta(a1, a2, scheme, 0.f, w);
            compressor->CompressDelta(a2, a1, scheme, 0.f, w);
            compressor->CompressFull(a1, scheme, 0.f, w);
            compressor->CompressFull(a2, scheme, 0.f, w);
        }
        w.WriteAlignmentBits();
        w.Flush();

        StringStream ss;
        BitReader r(buf, w.GetBytesWritten());
        for (uint32 i = 0; i < n && !r.IsOverflowed(); ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            Any ar{ T{} };

            T v1 = items[i].v1;
            T v2 = items[i].v2;
            T vq;

            using long_long = long long;
            compressor->DecompressDelta(a1, ar, scheme, r);
            vq = ar.Get<T>();
            ss << "original=" << v2 << " compressed=" << vq;
            TEST_VERIFY_WITH_MESSAGE(vq == v2, Format("tag=%d n=%u %s", tag, i, ss.str().c_str()));
            ss.clear();

            compressor->DecompressDelta(a2, ar, scheme, r);
            vq = ar.Get<T>();
            ss << "original=" << v1 << " compressed=" << vq;
            TEST_VERIFY_WITH_MESSAGE(v1 == v1, Format("tag=%d n=%u %s", tag, i, ss.str().c_str()));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<T>();
            ss << "original=" << v1 << " compressed=" << vq;
            TEST_VERIFY_WITH_MESSAGE(vq == v1, Format("tag=%d n=%u %s", tag, i, ss.str().c_str()));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<T>();
            ss << "original=" << v2 << " compressed=" << vq;
            TEST_VERIFY_WITH_MESSAGE(vq == v2, Format("tag=%d n=%u %s", tag, i, ss.str().c_str()));
        }
        TEST_VERIFY(!r.IsOverflowed());
    }

    DAVA_TEST (VarIntTest)
    {
        uint8 buf[1000];

        BitWriter w(buf, 1000);

        uint16 ui16[] = { 0, 5, 0x80, 0x3300, 0x8000, 0xFFFF };
        int16 i16[] = { 0, -1, -128, -256, 10'000, -10'000 };
        uint32 ui32[] = { 0, 5, 0x80, 0x3300, 0x8000, 0x453412, 0xFFFF'FFFF };
        int32 i32[] = { 0, -1, -128, -256, 10'000, -10'000, 100'000, -100'000 };
        uint64 ui64[] = { 0, 5, 0x80, 0x3300, 0x8000, 0x453412, 0x4566'3445'3412, 0xFFFF'FFFF'FFFF'FFFF };
        int64 i64[] = { 0, -1, -128, -256, 10'000, -10'000, 100'000, -100'000, 234'478'444'345, -234'478'444'345 };

        for (uint16 v : ui16)
        {
            CompressionUtils::CompressVarInt(v, w);
        }
        for (int16 v : i16)
        {
            CompressionUtils::CompressVarInt(v, w);
        }
        for (uint32 v : ui32)
        {
            CompressionUtils::CompressVarInt(v, w);
        }
        for (int32 v : i32)
        {
            CompressionUtils::CompressVarInt(v, w);
        }
        for (uint64 v : ui64)
        {
            CompressionUtils::CompressVarInt(v, w);
        }
        for (int64 v : i64)
        {
            CompressionUtils::CompressVarInt(v, w);
        }

        w.WriteAlignmentBits();
        w.Flush();

        BitReader r(buf, w.GetBytesWritten());
        for (uint16 v : ui16)
        {
            uint16 x = CompressionUtils::DecompressVarInt<uint16>(r);
            TEST_VERIFY(x == v);
        }
        for (int16 v : i16)
        {
            int16 x = CompressionUtils::DecompressVarInt<int16>(r);
            TEST_VERIFY(x == v);
        }
        for (uint32 v : ui32)
        {
            uint32 x = CompressionUtils::DecompressVarInt<uint32>(r);
            TEST_VERIFY(x == v);
        }
        for (int32 v : i32)
        {
            int32 x = CompressionUtils::DecompressVarInt<int32>(r);
            TEST_VERIFY(x == v);
        }
        for (uint64 v : ui64)
        {
            uint64 x = CompressionUtils::DecompressVarInt<uint64>(r);
            TEST_VERIFY(x == v);
        }
        for (int64 v : i64)
        {
            int64 x = CompressionUtils::DecompressVarInt<int64>(r);
            TEST_VERIFY(x == v);
        }
    }

    DAVA_TEST (QuaternionCompressorTest)
    {
        const CompressTestItem<Quaternion> testItems[] = {
            { Quaternion{ 0.000000f, 0.000000f, 0.000000f, 1.000000f }, Quaternion{ -0.000000f, 0.000000f, 0.996195f, -0.087156f } },
            { Quaternion{ -0.000000f, -0.000000f, 0.798635f, 0.601815f }, Quaternion{ -0.000000f, 0.000000f, 0.814115f, 0.580703f } },
            { Quaternion{ -0.000000f, -0.000000f, 0.909961f, 0.414693f }, Quaternion{ -0.000000f, -0.000000f, 0.920505f, 0.390731f } },
            { Quaternion{ -0.000000f, -0.000000f, 0.987688f, 0.156435f }, Quaternion{ -0.000000f, 0.000000f, 0.990268f, 0.139173f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.771625f, -0.636078f }, Quaternion{ -0.000000f, 0.000000f, 0.754710f, -0.656059f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.824126f, -0.566406f }, Quaternion{ -0.000000f, 0.000000f, 0.809017f, -0.587785f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.874620f, 0.484810f }, Quaternion{ -0.000000f, -0.000000f, 0.887011f, 0.461749f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.920505f, -0.390731f }, Quaternion{ -0.000000f, 0.000000f, 0.909961f, -0.414693f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.942642f, -0.333807f }, Quaternion{ -0.000000f, 0.000000f, 0.933581f, -0.358368f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.972370f, -0.233445f }, Quaternion{ -0.000000f, 0.000000f, 0.965926f, -0.258819f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.989016f, -0.147809f }, Quaternion{ -0.000000f, 0.000000f, 0.984808f, -0.173648f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.996917f, -0.078459f }, Quaternion{ -0.000000f, 0.000000f, 0.994522f, -0.104528f } },
            { Quaternion{ -0.000000f, 0.000000f, 0.999848f, 0.017453f }, Quaternion{ -0.000000f, 0.000000f, 0.999962f, -0.008726f } },
            { Quaternion{ 0.000000f, -0.000000f, 0.165048f, 0.986286f }, Quaternion{ 0.000000f, -0.000000f, 0.190809f, 0.981627f } },
            { Quaternion{ 0.000000f, -0.000000f, 0.430511f, 0.902585f }, Quaternion{ 0.000000f, -0.000000f, 0.453990f, 0.891007f } },
            { Quaternion{ 0.000000f, -0.000000f, 0.656059f, 0.754710f }, Quaternion{ -0.000000f, 0.000000f, 0.675590f, 0.737277f } },
            { Quaternion{ 0.000000f, 0.000000f, -0.113204f, -0.993572f }, Quaternion{ 0.000000f, 0.000000f, -0.130527f, -0.991445f } },
            { Quaternion{ 0.000000f, 0.000000f, -0.241922f, -0.970296f }, Quaternion{ 0.000000f, 0.000000f, -0.258820f, -0.965926f } },
            { Quaternion{ 0.000000f, 0.000000f, -0.358369f, -0.933580f }, Quaternion{ 0.000000f, 0.000000f, -0.382684f, -0.923879f } },
            { Quaternion{ 0.000000f, 0.000000f, -0.469472f, -0.882947f }, Quaternion{ 0.000000f, 0.000000f, -0.484810f, -0.874619f } },
            { Quaternion{ 0.000000f, 0.000000f, -0.573577f, -0.819152f }, Quaternion{ 0.000000f, 0.000000f, -0.594823f, -0.803856f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.061049f, 0.998135f }, Quaternion{ 0.000000f, 0.000000f, 0.078459f, 0.996917f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.147809f, 0.989016f }, Quaternion{ 0.000000f, -0.000000f, 0.165048f, 0.986286f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.224951f, 0.974370f }, Quaternion{ 0.000000f, 0.000000f, 0.241922f, 0.970296f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.366501f, 0.930418f }, Quaternion{ 0.000000f, -0.000000f, 0.390731f, 0.920505f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.446198f, 0.894934f }, Quaternion{ 0.000000f, 0.000000f, 0.461749f, 0.887011f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.522498f, -0.852640f }, Quaternion{ 0.000000f, 0.000000f, 0.500000f, -0.866026f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.587785f, 0.809017f }, Quaternion{ 0.000000f, -0.000000f, 0.608761f, 0.793353f } },
            { Quaternion{ 0.000000f, 0.000000f, 0.669131f, -0.743145f }, Quaternion{ 0.000000f, 0.000000f, 0.649448f, -0.760406f } },
            { Quaternion{ 0.000001f, 0.000000f, -0.694659f, -0.719339f }, Quaternion{ 0.000001f, 0.000000f, -0.707107f, -0.707106f } },

        };
        {
            M::QuaternionQuantizeParam qparam(0.01f);
            QuaternionQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
        {
            M::QuaternionQuantizeParam qparam(0.001f);
            QuaternionQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
        {
            M::QuaternionQuantizeParam qparam(0.0001f);
            QuaternionQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
        {
            M::QuaternionQuantizeParam qparam(0.00001f);
            QuaternionQuantizer(__LINE__, testItems, COUNT_OF(testItems), &qparam);
        }
    }

    void QuaternionQuantizer(int tag, const CompressTestItem<Quaternion>* items, uint32 n, const M::QuaternionQuantizeParam* qparam)
    {
        uint8 buf[1000];
        CompressionScheme scheme = CompressionUtils::MakeCompressionScheme(qparam);
        float32 precision = qparam->precision;
        const CompressorInterface* compressor = CompressionUtils::GetTypeCompressor(Type::Instance<Quaternion>());

        BitWriter w(buf, 1000);
        for (uint32 i = 0; i < n; ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            compressor->CompressDelta(a1, a2, scheme, precision, w);
            compressor->CompressDelta(a2, a1, scheme, precision, w);
            compressor->CompressFull(a1, scheme, precision, w);
            compressor->CompressFull(a2, scheme, precision, w);
        }
        w.WriteAlignmentBits();
        w.Flush();

        auto formatQ = [](const Quaternion& q) -> String {
            char buf[200];
            snprintf(buf, 200, "{%f,%f,%f,%f}", q.x, q.y, q.z, q.w);
            return String(buf);
        };

        auto checkDelta = [](const Quaternion& q1, const Quaternion& q2, float32 precision, Quaternion& d) -> bool {
            d.x = std::abs(q2.x - q1.x);
            d.y = std::abs(q2.y - q1.y);
            d.z = std::abs(q2.z - q1.z);
            d.w = std::abs(q2.w - q1.w);
            return d.x < precision && d.y < precision && d.z < precision && d.w < precision;
        };

        BitReader r(buf, w.GetBytesWritten());
        for (uint32 i = 0; i < n && !r.IsOverflowed(); ++i)
        {
            Any a1{ items[i].v1 };
            Any a2{ items[i].v2 };
            Any ar{ Quaternion{} };

            Quaternion v1 = items[i].v1;
            Quaternion v2 = items[i].v2;
            Quaternion vq;
            Quaternion diff;
            bool isEqual;

            compressor->DecompressDelta(a1, ar, scheme, r);
            vq = ar.Get<Quaternion>();
            isEqual = checkDelta(vq, v2, precision, diff);
            TEST_VERIFY_WITH_MESSAGE(isEqual,
                                     Format("tag=%d n=%u original=%s quantized=%s diff=%s", tag, i, formatQ(v2).c_str(), formatQ(vq).c_str(), formatQ(diff).c_str()));

            compressor->DecompressDelta(a2, ar, scheme, r);
            vq = ar.Get<Quaternion>();
            isEqual = checkDelta(vq, v1, precision, diff);
            TEST_VERIFY_WITH_MESSAGE(isEqual,
                                     Format("tag=%d n=%u original=%s quantized=%s diff=%s", tag, i, formatQ(v1).c_str(), formatQ(vq).c_str(), formatQ(diff).c_str()));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<Quaternion>();
            isEqual = checkDelta(vq, v1, precision, diff);
            TEST_VERIFY_WITH_MESSAGE(isEqual,
                                     Format("tag=%d n=%u original=%s quantized=%s diff=%s", tag, i, formatQ(v1).c_str(), formatQ(vq).c_str(), formatQ(diff).c_str()));

            compressor->DecompressFull(ar, scheme, r);
            vq = ar.Get<Quaternion>();
            isEqual = checkDelta(vq, v2, precision, diff);
            TEST_VERIFY_WITH_MESSAGE(isEqual,
                                     Format("tag=%d n=%u original=%s quantized=%s diff=%s", tag, i, formatQ(v2).c_str(), formatQ(vq).c_str(), formatQ(diff).c_str()));
        }
        TEST_VERIFY(!r.IsOverflowed());
    }
};
