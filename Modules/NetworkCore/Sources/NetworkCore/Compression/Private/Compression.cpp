#include "NetworkCore/Compression/Compression.h"

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Compression/AnyCompressor.h"
#include "NetworkCore/Compression/FixedVectorCompressor.h"
#include "NetworkCore/Compression/FloatCompressor.h"
#include "NetworkCore/Compression/IntegralCompressor.h"
#include "NetworkCore/Compression/Matrix4Compressor.h"
#include "NetworkCore/Compression/QuaternionCompressor.h"
#include "NetworkCore/Compression/StringCompressor.h"
#include "NetworkCore/Compression/Vector2Compressor.h"
#include "NetworkCore/Compression/Vector3Compressor.h"

#include <Base/FastName.h>
#include <Base/FixedVector.h>
#include <Base/Type.h>
#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Math/Quaternion.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
// clang-format off
const std::array<IntCompressionRecord, 29> intCompressionRecords = {
    IntCompressionRecord{},
    IntCompressionRecord{  2, 0x00000001 }, // [-1, 1]
    IntCompressionRecord{  3, 0x00000003 }, // [-3, 3]
    IntCompressionRecord{  4, 0x00000007 }, // [-7, 7]
    IntCompressionRecord{  5, 0x0000000F }, // [-15, 15]
    IntCompressionRecord{  6, 0x0000001F }, // [-31, 31]
    IntCompressionRecord{  7, 0x0000003F }, // [-63, 63]
    IntCompressionRecord{  8, 0x0000007F }, // [-127, 127]
    IntCompressionRecord{  9, 0x000000FF }, // [-255, 255]
    IntCompressionRecord{ 10, 0x000001FF }, // [-511, 511]
    IntCompressionRecord{ 11, 0x000003FF }, // [-1023, 1023]
    IntCompressionRecord{ 12, 0x000007FF }, // [-2047, 2047]
    IntCompressionRecord{ 13, 0x00000FFF }, // [-4095, 4095]
    IntCompressionRecord{ 14, 0x00001FFF }, // [-8191, 8191]
    IntCompressionRecord{ 15, 0x00003FFF }, // [-16383, 16383]
    IntCompressionRecord{ 16, 0x00007FFF }, // [-32767, 32767]
    IntCompressionRecord{ 17, 0x0000FFFF }, // [-65535, 65535]
    IntCompressionRecord{ 18, 0x0001FFFF }, // [-131071, 131071]
    IntCompressionRecord{ 19, 0x0003FFFF }, // [-262143, 262143]
    IntCompressionRecord{ 20, 0x0007FFFF }, // [-524287, 524287]
    IntCompressionRecord{ 21, 0x000FFFFF }, // [-1048575, 1048575]
    IntCompressionRecord{ 22, 0x001FFFFF }, // [-2097151, 2097151]
    IntCompressionRecord{ 23, 0x003FFFFF }, // [-4194303, 4194303]
    IntCompressionRecord{ 24, 0x007FFFFF }, // [-8388607, 8388607]
    IntCompressionRecord{ 25, 0x00FFFFFF }, // [-16777215, 16777215]
    IntCompressionRecord{ 26, 0x01FFFFFF }, // [-33554431, 33554431]
    IntCompressionRecord{ 27, 0x03FFFFFF }, // [-67108863, 67108863]
    IntCompressionRecord{ 28, 0x07FFFFFF }, // [-134217727, 134217727]
    IntCompressionRecord{ 29, 0x0FFFFFFF }, // [-268435455, 268435455]
};
const std::array<Int64CompressionRecord, 61> int64CompressionRecords = {
    Int64CompressionRecord{},
    Int64CompressionRecord{  2, 0x0000000000000001 }, // [-1, 1]
    Int64CompressionRecord{  3, 0x0000000000000003 }, // [-3, 3]
    Int64CompressionRecord{  4, 0x0000000000000007 }, // [-7, 7]
    Int64CompressionRecord{  5, 0x000000000000000F }, // [-15, 15]
    Int64CompressionRecord{  6, 0x000000000000001F }, // [-31, 31]
    Int64CompressionRecord{  7, 0x000000000000003F }, // [-63, 63]
    Int64CompressionRecord{  8, 0x000000000000007F }, // [-127, 127]
    Int64CompressionRecord{  9, 0x00000000000000FF }, // [-255, 255]
    Int64CompressionRecord{ 10, 0x00000000000001FF }, // [-511, 511]
    Int64CompressionRecord{ 11, 0x00000000000003FF }, // [-1023, 1023]
    Int64CompressionRecord{ 12, 0x00000000000007FF }, // [-2047, 2047]
    Int64CompressionRecord{ 13, 0x0000000000000FFF }, // [-4095, 4095]
    Int64CompressionRecord{ 14, 0x0000000000001FFF }, // [-8191, 8191]
    Int64CompressionRecord{ 15, 0x0000000000003FFF }, // [-16383, 16383]
    Int64CompressionRecord{ 16, 0x0000000000007FFF }, // [-32767, 32767]
    Int64CompressionRecord{ 17, 0x000000000000FFFF }, // [-65535, 65535]
    Int64CompressionRecord{ 18, 0x000000000001FFFF }, // [-131071, 131071]
    Int64CompressionRecord{ 19, 0x000000000003FFFF }, // [-262143, 262143]
    Int64CompressionRecord{ 20, 0x000000000007FFFF }, // [-524287, 524287]
    Int64CompressionRecord{ 21, 0x00000000000FFFFF }, // [-1048575, 1048575]
    Int64CompressionRecord{ 22, 0x00000000001FFFFF }, // [-2097151, 2097151]
    Int64CompressionRecord{ 23, 0x00000000003FFFFF }, // [-4194303, 4194303]
    Int64CompressionRecord{ 24, 0x00000000007FFFFF }, // [-8388607, 8388607]
    Int64CompressionRecord{ 25, 0x0000000000FFFFFF }, // [-16777215, 16777215]
    Int64CompressionRecord{ 26, 0x0000000001FFFFFF }, // [-33554431, 33554431]
    Int64CompressionRecord{ 27, 0x0000000003FFFFFF }, // [-67108863, 67108863]
    Int64CompressionRecord{ 28, 0x0000000007FFFFFF }, // [-134217727, 134217727]
    Int64CompressionRecord{ 29, 0x000000000FFFFFFF }, // [-268435455, 268435455]
    Int64CompressionRecord{ 30, 0x000000001FFFFFFF }, // [-536870911, 536870911]
    Int64CompressionRecord{ 31, 0x000000003FFFFFFF }, // [-1073741823, 1073741823]
    Int64CompressionRecord{ 32, 0x000000007FFFFFFF }, // [-2147483647, 2147483647]
    Int64CompressionRecord{ 33, 0x00000000FFFFFFFF }, // [-4294967295, 4294967295]
    Int64CompressionRecord{ 34, 0x00000001FFFFFFFF }, // [-8589934591, 8589934591]
    Int64CompressionRecord{ 35, 0x00000003FFFFFFFF }, // [-17179869183, 17179869183]
    Int64CompressionRecord{ 36, 0x00000007FFFFFFFF }, // [-34359738367, 34359738367]
    Int64CompressionRecord{ 37, 0x0000000FFFFFFFFF }, // [-68719476735, 68719476735]
    Int64CompressionRecord{ 38, 0x0000001FFFFFFFFF }, // [-137438953471, 137438953471]
    Int64CompressionRecord{ 39, 0x0000003FFFFFFFFF }, // [-274877906943, 274877906943]
    Int64CompressionRecord{ 40, 0x0000007FFFFFFFFF }, // [-549755813887, 549755813887]
    Int64CompressionRecord{ 41, 0x000000FFFFFFFFFF }, // [-1099511627775, 1099511627775]
    Int64CompressionRecord{ 42, 0x000001FFFFFFFFFF }, // [-2199023255551, 2199023255551]
    Int64CompressionRecord{ 43, 0x000003FFFFFFFFFF }, // [-4398046511103, 4398046511103]
    Int64CompressionRecord{ 44, 0x000007FFFFFFFFFF }, // [-8796093022207, 8796093022207]
    Int64CompressionRecord{ 45, 0x00000FFFFFFFFFFF }, // [-17592186044415, 17592186044415]
    Int64CompressionRecord{ 46, 0x00001FFFFFFFFFFF }, // [-35184372088831, 35184372088831]
    Int64CompressionRecord{ 47, 0x00003FFFFFFFFFFF }, // [-70368744177663, 70368744177663]
    Int64CompressionRecord{ 48, 0x00007FFFFFFFFFFF }, // [-140737488355327, 140737488355327]
    Int64CompressionRecord{ 49, 0x0000FFFFFFFFFFFF }, // [-281474976710655, 281474976710655]
    Int64CompressionRecord{ 50, 0x0001FFFFFFFFFFFF }, // [-562949953421311, 562949953421311]
    Int64CompressionRecord{ 51, 0x0003FFFFFFFFFFFF }, // [-1125899906842623, 1125899906842623]
    Int64CompressionRecord{ 52, 0x0007FFFFFFFFFFFF }, // [-2251799813685247, 2251799813685247]
    Int64CompressionRecord{ 53, 0x000FFFFFFFFFFFFF }, // [-4503599627370495, 4503599627370495]
    Int64CompressionRecord{ 54, 0x001FFFFFFFFFFFFF }, // [-9007199254740991, 9007199254740991]
    Int64CompressionRecord{ 55, 0x003FFFFFFFFFFFFF }, // [-18014398509481983, 18014398509481983]
    Int64CompressionRecord{ 56, 0x007FFFFFFFFFFFFF }, // [-36028797018963967, 36028797018963967]
    Int64CompressionRecord{ 57, 0x00FFFFFFFFFFFFFF }, // [-72057594037927935, 72057594037927935]
    Int64CompressionRecord{ 58, 0x01FFFFFFFFFFFFFF }, // [-144115188075855871, 144115188075855871]
    Int64CompressionRecord{ 59, 0x03FFFFFFFFFFFFFF }, // [-288230376151711743, 288230376151711743]
    Int64CompressionRecord{ 60, 0x07FFFFFFFFFFFFFF }, // [-576460752303423487, 576460752303423487]
    Int64CompressionRecord{ 61, 0x0FFFFFFFFFFFFFFF }, // [-1152921504606846975, 1152921504606846975]
};
const std::array<FracCompressionRecord, 18> fracCompressionRecords = {
    FracCompressionRecord{},
    FracCompressionRecord{  4, 0.066667f },
    FracCompressionRecord{  5, 0.032258f },
    FracCompressionRecord{  6, 0.015873f },
    FracCompressionRecord{  7, 0.007874f },
    FracCompressionRecord{  8, 0.003922f },
    FracCompressionRecord{  9, 0.001957f },
    FracCompressionRecord{ 10, 0.000978f },
    FracCompressionRecord{ 11, 0.000489f },
    FracCompressionRecord{ 12, 0.000244f },
    FracCompressionRecord{ 13, 0.000122f },
    FracCompressionRecord{ 14, 0.000061f },
    FracCompressionRecord{ 15, 0.000031f },
    FracCompressionRecord{ 16, 0.000015f },
    FracCompressionRecord{ 17, 0.000008f },
    FracCompressionRecord{ 18, 0.000004f },
    FracCompressionRecord{ 19, 0.000002f },
    FracCompressionRecord{ 20, 0.000001f },
};
const std::array<FracCompressionRecord, 12> quaternionCompressionRecords = {
    FracCompressionRecord{},
    FracCompressionRecord{  9, 0.00276754f },
    FracCompressionRecord{ 10, 0.00138242f },
    FracCompressionRecord{ 11, 0.00069087f },
    FracCompressionRecord{ 12, 0.00034535f },
    FracCompressionRecord{ 13, 0.00017265f },
    FracCompressionRecord{ 14, 0.00008632f },
    FracCompressionRecord{ 15, 0.00004316f },
    FracCompressionRecord{ 16, 0.00002158f },
    FracCompressionRecord{ 17, 0.00001079f },
    FracCompressionRecord{ 18, 0.00000539f },
    FracCompressionRecord{ 19, 0.00000270f },
};
// clang-format on

uint32 FindSuitableRecordIndex(float32 precision, const FracCompressionRecord* records, size_t size)
{
    for (size_t i = 1; i < size; ++i)
    {
        if (precision >= records[i].precision)
            return static_cast<uint32>(i);
    }
    return 0;
}

uint32 FindSuitableRecordIndex(uint32 range, const IntCompressionRecord* records, size_t size)
{
    for (size_t i = 1; i < size; ++i)
    {
        if (range <= records[i].range)
            return static_cast<uint32>(i);
    }
    return 0;
}

uint32 FindSuitableRecordIndex(uint64 range, const Int64CompressionRecord* records, size_t size)
{
    for (size_t i = 1; i < size; ++i)
    {
        if (range <= records[i].range)
            return static_cast<uint32>(i);
    }
    return 0;
}

CompressionScheme ComposeCompressionScheme(uint32 fullRangeIndex, uint32 deltaRangeIndex, uint32 fracIndex)
{
    return CompressionScheme{ fullRangeIndex | (deltaRangeIndex << 8) | (fracIndex << 16) };
}

CompressionScheme CompressionUtils::MakeCompressionScheme(const Metas::FloatQuantizeParam* param)
{
    DVASSERT(param != nullptr);
    DVASSERT(0.f < param->deltaRange && param->deltaRange <= param->fullRange);
    DVASSERT(0.f < param->precision && param->precision < 1.f);

    uint32 fullRangeIndex = FindSuitableRecordIndex(static_cast<uint32>(param->fullRange), intCompressionRecords.data(), intCompressionRecords.size());
    uint32 deltaRangeIndex = FindSuitableRecordIndex(static_cast<uint32>(param->deltaRange), intCompressionRecords.data(), intCompressionRecords.size());
    uint32 fracIndex = FindSuitableRecordIndex(param->precision, fracCompressionRecords.data(), fracCompressionRecords.size());

    DVASSERT(fullRangeIndex > 0 && deltaRangeIndex > 0 && fracIndex > 0);

    const IntCompressionRecord& fullRangeRecord = intCompressionRecords[fullRangeIndex];
    const IntCompressionRecord& deltaRangeRecord = intCompressionRecords[deltaRangeIndex];
    const FracCompressionRecord& fracRecord = fracCompressionRecords[fracIndex];

    DVASSERT(fullRangeRecord.bits <= 24 && deltaRangeRecord.bits <= 24);
    DVASSERT(fullRangeRecord.bits + fracRecord.bits < 30);

    return ComposeCompressionScheme(fullRangeIndex, deltaRangeIndex, fracIndex);
}

CompressionScheme CompressionUtils::MakeCompressionScheme(const Metas::QuaternionQuantizeParam* param)
{
    DVASSERT(param != nullptr);

    uint32 index = FindSuitableRecordIndex(param->precision, quaternionCompressionRecords.data(), quaternionCompressionRecords.size());
    DVASSERT(index > 0);

    return ComposeCompressionScheme(0, 0, index);
}

CompressionScheme CompressionUtils::MakeCompressionScheme(const Metas::IntCompressParam* param)
{
    DVASSERT(param != nullptr);
    DVASSERT(0 < param->deltaRange && param->deltaRange <= param->fullRange);

    uint32 fullRangeIndex = FindSuitableRecordIndex(param->fullRange, intCompressionRecords.data(), intCompressionRecords.size());
    uint32 deltaRangeIndex = FindSuitableRecordIndex(param->deltaRange, intCompressionRecords.data(), intCompressionRecords.size());

    DVASSERT(fullRangeIndex > 0 && deltaRangeIndex > 0);

    return ComposeCompressionScheme(fullRangeIndex, deltaRangeIndex, 0);
}

CompressionScheme CompressionUtils::MakeCompressionScheme(const Metas::Int64CompressParam* param)
{
    DVASSERT(param != nullptr);
    DVASSERT(0 < param->deltaRange && param->deltaRange <= param->fullRange);

    uint32 fullRangeIndex = FindSuitableRecordIndex(param->fullRange, int64CompressionRecords.data(), int64CompressionRecords.size());
    uint32 deltaRangeIndex = FindSuitableRecordIndex(param->deltaRange, int64CompressionRecords.data(), int64CompressionRecords.size());

    DVASSERT(fullRangeIndex > 0 && deltaRangeIndex > 0);

    return ComposeCompressionScheme(fullRangeIndex, deltaRangeIndex, 0);
}

CompressionScheme CompressionUtils::GetFloatCompressionSchemeFromMeta(const ReflectedMeta* meta)
{
    DVASSERT(meta != nullptr);

    const Metas::FloatQuantizeParam* param = meta->GetMeta<Meta<Metas::FloatQuantizeParam>>();
    if (param != nullptr)
    {
        DVASSERT(meta->GetMeta<Meta<Metas::QuaternionQuantizeParam>>() == nullptr, "FloatQuantizeParam and QuaternionQuantizeParam metas should not be specified together");
        return MakeCompressionScheme(param);
    }
    return 0;
}

CompressionScheme CompressionUtils::GetQuaternionCompressionSchemeFromMeta(const ReflectedMeta* meta)
{
    DVASSERT(meta != nullptr);

    const Metas::QuaternionQuantizeParam* param = meta->GetMeta<Meta<Metas::QuaternionQuantizeParam>>();
    if (param != nullptr)
    {
        DVASSERT(meta->GetMeta<Meta<Metas::FloatQuantizeParam>>() == nullptr, "FloatQuantizeParam and QuaternionQuantizeParam metas should not be specified together");
        return MakeCompressionScheme(param);
    }
    return 0;
}

CompressionScheme CompressionUtils::GetIntCompressionSchemeFromMeta(const ReflectedMeta* meta)
{
    DVASSERT(meta != nullptr);

    const Metas::IntCompressParam* param = meta->GetMeta<Meta<Metas::IntCompressParam>>();
    if (param != nullptr)
    {
        return MakeCompressionScheme(param);
    }
    return 0;
}

CompressionScheme CompressionUtils::GetInt64CompressionSchemeFromMeta(const ReflectedMeta* meta)
{
    DVASSERT(meta != nullptr);

    const Metas::Int64CompressParam* param = meta->GetMeta<Meta<Metas::Int64CompressParam>>();
    if (param != nullptr)
    {
        return MakeCompressionScheme(param);
    }
    return 0;
}

float32 CompressionUtils::GetComparePrecisionFromMeta(const ReflectedMeta* meta, float32 defaultPrecision)
{
    DVASSERT(meta != nullptr);

    float32 precision = defaultPrecision;

    const Metas::ComparePrecision* c = meta->GetMeta<Meta<Metas::ComparePrecision>>();
    if (c != nullptr)
    {
        precision = c->precision;
    }
    else
    {
        const Metas::FloatQuantizeParam* qf = meta->GetMeta<Meta<Metas::FloatQuantizeParam>>();
        const Metas::QuaternionQuantizeParam* qq = meta->GetMeta<Meta<Metas::QuaternionQuantizeParam>>();
        if (qf != nullptr)
        {
            uint32 index = FindSuitableRecordIndex(qf->precision, fracCompressionRecords.data(), fracCompressionRecords.size());
            precision = fracCompressionRecords[index].precision;
        }
        else if (qq != nullptr)
        {
            uint32 index = FindSuitableRecordIndex(qq->precision, quaternionCompressionRecords.data(), quaternionCompressionRecords.size());
            precision = quaternionCompressionRecords[index].precision;
        }
    }

    DVASSERT(0.f <= precision && precision < 1.f);
    return precision;
}

const FracCompressionRecord* CompressionUtils::GetFracRecord(CompressionScheme scheme)
{
    const size_t index = (scheme >> 16) & 0xFF;
    DVASSERT(0 < index && index < fracCompressionRecords.size());
    return &fracCompressionRecords[index];
}

const FracCompressionRecord* CompressionUtils::GetQuaternionRecord(CompressionScheme scheme)
{
    const size_t index = (scheme >> 16) & 0xFF;
    DVASSERT(0 < index && index < quaternionCompressionRecords.size());
    return &quaternionCompressionRecords[index];
}

const IntCompressionRecord* CompressionUtils::GetFullRangeRecord(CompressionScheme scheme)
{
    const size_t index = scheme & 0xFF;
    DVASSERT(0 < index && index < intCompressionRecords.size());
    return &intCompressionRecords[index];
}

const IntCompressionRecord* CompressionUtils::GetDeltaRangeRecord(CompressionScheme scheme)
{
    const size_t index = (scheme >> 8) & 0xFF;
    DVASSERT(0 < index && index < intCompressionRecords.size());
    return &intCompressionRecords[index];
}

const Int64CompressionRecord* CompressionUtils::GetFullRange64Record(CompressionScheme scheme)
{
    const size_t index = scheme & 0xFF;
    DVASSERT(0 < index && index < int64CompressionRecords.size());
    return &int64CompressionRecords[index];
}

const Int64CompressionRecord* CompressionUtils::GetDeltaRange64Record(CompressionScheme scheme)
{
    const size_t index = (scheme >> 8) & 0xFF;
    DVASSERT(0 < index && index < int64CompressionRecords.size());
    return &int64CompressionRecords[index];
}

float32 CompressionUtils::GetFloatDeltaPrecision(CompressionScheme scheme, float32 deltaPrecision)
{
    if (scheme != 0)
    {
        const FracCompressionRecord* record = CompressionUtils::GetFracRecord(scheme);
        return record->precision;
    }
    return deltaPrecision;
}

float32 CompressionUtils::GetQuaternionDeltaPrecision(CompressionScheme scheme, float32 deltaPrecision)
{
    if (scheme != 0)
    {
        const FracCompressionRecord* record = CompressionUtils::GetQuaternionRecord(scheme);
        return record->precision;
    }
    return deltaPrecision;
}

void CompressionUtils::CompressFloat(float32 value, float32 deltaPrecision, const IntCompressionRecord* intRecord, const FracCompressionRecord* fracRecord, BitWriter& writer)
{
    DVASSERT((intRecord != nullptr && fracRecord != nullptr) || (intRecord == nullptr && fracRecord == nullptr));

    if (std::abs(value) > deltaPrecision)
    {
        writer.WriteBits(1, 1);
        if (intRecord != nullptr)
        {
            const float32 absolute = std::abs(value);
            const float32 truncated = std::trunc(absolute);
            const float32 fractional = absolute - truncated;
            DVASSERT(truncated <= static_cast<float32>(intRecord->range));

            // Write sign bit to cover values such as -0.3445
            writer.WriteBits(std::signbit(value), 1);
            uint32 intPart = static_cast<uint32>(truncated);
            uint32 fracPart = static_cast<uint32>(fractional / fracRecord->precision);

            //uint32 intPart = static_cast<uint32>(value) + intRecord->range;
            //uint32 fracPart = static_cast<uint32>(fractional / fracRecord->precision);
            writer.WriteBits(intPart, intRecord->bits);
            writer.WriteBits(fracPart, fracRecord->bits);
        }
        else
        {
            writer.WriteBits(*reinterpret_cast<uint32*>(&value), 32);
        }
    }
    else
    {
        writer.WriteBits(0, 1);
    }

#if defined(COMPRESSOR_DEBUG_ENABLED)
    if (intRecord != nullptr)
    {
        writer.WriteBits(*reinterpret_cast<uint32*>(&value), 32);
    }
#endif
}

float32 CompressionUtils::DecompressFloat(const IntCompressionRecord* intRecord, const FracCompressionRecord* fracRecord, BitReader& reader)
{
    DVASSERT((intRecord != nullptr && fracRecord != nullptr) || (intRecord == nullptr && fracRecord == nullptr));

    float32 result = 0.f;
    if (reader.ReadBits(1) != 0)
    {
        if (intRecord != nullptr)
        {
            uint32 sign = reader.ReadBits(1);
            uint32 intPart = reader.ReadBits(intRecord->bits);
            uint32 fracPart = reader.ReadBits(fracRecord->bits);

            result = static_cast<float32>(intPart);
            result += static_cast<float32>(fracPart) * fracRecord->precision;
            if (sign)
                result = -result;

            //result = static_cast<float32>(static_cast<int32>(intPart - intRecord->range));
            //result += static_cast<float32>(fracPart) * fracRecord->precision;
        }
        else
        {
            uint32 x = reader.ReadBits(32);
            result = *reinterpret_cast<float32*>(&x);
        }
    }

#if defined(COMPRESSOR_DEBUG_ENABLED)
    if (intRecord != nullptr)
    {
        uint32 x = reader.ReadBits(32);
        float32 c = *reinterpret_cast<float32*>(&x);
        float32 delta = std::abs(c - result);
        DVASSERT(delta < fracRecord->precision);
    }
#endif
    return result;
}

const float quaternionMinimum = -0.7071067811865476f; // -1 / sqrt(2)
const float quaternionMaximum = 0.7071067811865476f; // 1 / sqrt(2)

void CompressionUtils::CompressQuaternion(const Quaternion& q, const FracCompressionRecord* qRecord, BitWriter& writer)
{
    auto maxComponentAt = std::max_element(std::begin(q.data), std::end(q.data), [](const float32 l, const float32 r) -> bool {
        return std::abs(l) < std::abs(r);
    });
    const uint32 maxComponentIndex = static_cast<uint32>(std::distance(std::begin(q.data), maxComponentAt));
    const uint32 maxComponentSign = std::signbit(q.data[maxComponentIndex]);

    writer.WriteBits(maxComponentIndex, 2);
    writer.WriteBits(maxComponentSign, 1);
    if (qRecord != nullptr)
    {
        const float32 scale = 1.f / qRecord->precision;
        for (uint32 i = 0, j = 0; i < 4; ++i)
        {
            if (i != maxComponentIndex)
            {
                float32 v = (q.data[i] - quaternionMinimum) / (quaternionMaximum - quaternionMinimum);
                v *= scale;
                v += .5f;

                writer.WriteBits(static_cast<uint32>(v), qRecord->bits);
                j += 1;
            }
        }
    }
    else
    {
        for (uint32 i = 0, j = 0; i < 4; ++i)
        {
            if (i != maxComponentIndex)
            {
                writer.WriteBits(*reinterpret_cast<const uint32*>(&q.data[i]), 32);
                j += 1;
            }
        }
    }

#if defined(COMPRESSOR_DEBUG_ENABLED)
    if (qRecord != nullptr)
    {
        CompressFloat(q.x, 0.f, nullptr, nullptr, writer);
        CompressFloat(q.y, 0.f, nullptr, nullptr, writer);
        CompressFloat(q.z, 0.f, nullptr, nullptr, writer);
        CompressFloat(q.w, 0.f, nullptr, nullptr, writer);
    }
#endif
}

Quaternion CompressionUtils::DecompressQuaternion(const FracCompressionRecord* qRecord, BitReader& reader)
{
    Quaternion q;
    float32 squaredSum = 0.f;
    const uint32 maxComponentIndex = reader.ReadBits(2);
    const uint32 maxComponentSign = reader.ReadBits(1);
    if (qRecord != nullptr)
    {
        const float32 inverseScale = qRecord->precision;
        for (uint32 i = 0; i < 3; ++i)
        {
            float32 v = static_cast<float32>(reader.ReadBits(qRecord->bits));
            v *= inverseScale;
            v *= (quaternionMaximum - quaternionMinimum);
            v += quaternionMinimum;

            q.data[i + (i >= maxComponentIndex)] = v;
            squaredSum += v * v;
        }
    }
    else
    {
        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 temp = reader.ReadBits(32);
            float32 v = *reinterpret_cast<float32*>(&temp);

            q.data[i + (i >= maxComponentIndex)] = v;
            squaredSum += v * v;
        }
    }

    q.data[maxComponentIndex] = std::sqrt(1.f - squaredSum);
    if (maxComponentSign)
    {
        q.data[maxComponentIndex] = -q.data[maxComponentIndex];
    }

#if defined(COMPRESSOR_DEBUG_ENABLED)
    if (qRecord != nullptr)
    {
        Quaternion c;
        c.x = CompressionUtils::DecompressFloat(nullptr, nullptr, reader);
        c.y = CompressionUtils::DecompressFloat(nullptr, nullptr, reader);
        c.z = CompressionUtils::DecompressFloat(nullptr, nullptr, reader);
        c.w = CompressionUtils::DecompressFloat(nullptr, nullptr, reader);

        float32 deltaX = std::abs(q.x - c.x);
        float32 deltaY = std::abs(q.y - c.x);
        float32 deltaZ = std::abs(q.z - c.x);
        float32 deltaW = std::abs(q.w - c.x);
        DVASSERT(deltaX < qRecord->precision || deltaY < qRecord->precision || deltaZ < qRecord->precision || deltaW < qRecord->precision);
    }
#endif
    return q;
}

uint32 GetTypeCompressorIndex();

const CompressorInterface* CompressionUtils::GetTypeCompressor(const Type* type)
{
    DVASSERT(type != nullptr);
    if (type->IsArray())
    {
        const Type* elemType = type->GetArrayElementType();
        const CompressorInterface* compressorInterface = static_cast<CompressorInterface*>(elemType->GetUserData(GetTypeCompressorIndex()));
        return compressorInterface;
    }
    else if (type->IsEnum())
    {
        const Type* intType = Type::Instance<int32>();
        const CompressorInterface* compressorInterface = static_cast<CompressorInterface*>(intType->GetUserData(GetTypeCompressorIndex()));
        return compressorInterface;
    }

    const CompressorInterface* compressorInterface = static_cast<CompressorInterface*>(type->GetUserData(GetTypeCompressorIndex()));
    return compressorInterface;
}

const CompressorInterface* CompressionUtils::GetTypeCompressor(const Any& any)
{
    return GetTypeCompressor(any.GetType());
}

} // namespace DAVA
