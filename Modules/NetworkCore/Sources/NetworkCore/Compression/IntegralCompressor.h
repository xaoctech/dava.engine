#pragma once

#include "NetworkCore/Compression/Compression.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Debug/DVAssert.h>
#include <Utils/StringFormat.h>

#include <type_traits>

namespace DAVA
{
class ReflectedMeta;
struct IntegerCompressionDescriptor;

template <typename T>
class IntegralCompressor
{
    // clang-format off
    static const bool is_type_allowed = std::is_enum<T>::value || std::is_integral<T>::value;
    static_assert(is_type_allowed, "Type is not allowed");
    // clang-format on

    template <bool, typename U>
    struct actual_type_selector
    {
        using type = std::underlying_type_t<U>;
    };

    template <typename U>
    struct actual_type_selector<false, U>
    {
        using type = U;
    };

    using ActualType = typename actual_type_selector<std::is_enum<T>::value, T>::type;

public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(T value1, T value2, float32 /*comparePrecision*/);

    static bool CompressDelta(T value1, T value2, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);
    static void CompressFull(T value, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);

    static void DecompressDelta(T baseValue, T* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(T* targetValue, CompressionScheme scheme, BitReader& reader);

private:
    template <typename X>
    using select_bool = const std::enable_if_t<std::is_same<X, bool>::value, X>*;
    template <typename X>
    using select_lt32bit = const std::enable_if_t<sizeof(X) < 8 && !std::is_same<X, bool>::value>*;
    template <typename X>
    using select_64bit = const std::enable_if_t<sizeof(X) == 8>*;

    template <typename X>
    using select_signed = const std::enable_if_t<std::is_signed<X>::value>*;
    template <typename X>
    using select_unsigned = const std::enable_if_t<std::is_unsigned<X>::value>*;

    template <typename W>
    static W Abs(W v, select_signed<W> = nullptr);
    template <typename W>
    static W Abs(W v, select_unsigned<W> = nullptr);

    template <typename W>
    static CompressionScheme GetCompressionSchemeImpl(const ReflectedMeta* meta, select_bool<W> = nullptr);
    template <typename W>
    static CompressionScheme GetCompressionSchemeImpl(const ReflectedMeta* meta, select_lt32bit<W> = nullptr);
    template <typename W>
    static CompressionScheme GetCompressionSchemeImpl(const ReflectedMeta* meta, select_64bit<W> = nullptr);

    // bool compression impl
    template <typename W>
    static bool CompressDeltaImpl(W value1, W value2, CompressionScheme scheme, BitWriter& writer, select_bool<W> = nullptr);
    template <typename W>
    static void CompressFullImpl(W value, CompressionScheme scheme, BitWriter& writer, select_bool<W> = nullptr);
    template <typename W>
    static W DecompressDeltaImpl(W baseValue, CompressionScheme scheme, BitReader& reader, select_bool<W> = nullptr);
    template <typename W>
    static W DecompressFullImpl(CompressionScheme scheme, BitReader& reader, select_bool<W> = nullptr);

    // 8bit, 16bit and 32bit integer compression impl
    template <typename W>
    static bool CompressDeltaImpl(W value1, W value2, CompressionScheme scheme, BitWriter& writer, select_lt32bit<W> = nullptr);
    template <typename W>
    static void CompressFullImpl(W value, CompressionScheme scheme, BitWriter& writer, select_lt32bit<W> = nullptr);
    template <typename W>
    static W DecompressDeltaImpl(W baseValue, CompressionScheme scheme, BitReader& reader, select_lt32bit<W> = nullptr);
    template <typename W>
    static W DecompressFullImpl(CompressionScheme scheme, BitReader& reader, select_lt32bit<W> = nullptr);

    // 64bit integer compression impl
    template <typename W>
    static bool CompressDeltaImpl(W value1, W value2, CompressionScheme scheme, BitWriter& writer, select_64bit<W> = nullptr);
    template <typename W>
    static void CompressFullImpl(W value, CompressionScheme scheme, BitWriter& writer, select_64bit<W> = nullptr);
    template <typename W>
    static W DecompressDeltaImpl(W baseValue, CompressionScheme scheme, BitReader& reader, select_64bit<W> = nullptr);
    template <typename W>
    static W DecompressFullImpl(CompressionScheme scheme, BitReader& reader, select_64bit<W> = nullptr);
};

template <typename T>
CompressionScheme IntegralCompressor<T>::GetCompressionScheme(const ReflectedMeta* meta)
{
    return GetCompressionSchemeImpl<ActualType>(meta);
}

template <typename T>
float32 IntegralCompressor<T>::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

template <typename T>
float32 IntegralCompressor<T>::GetComparePrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

template <typename T>
bool IntegralCompressor<T>::IsEqual(T value1, T value2, float32 /*comparePrecision*/)
{
    return value1 == value2;
}

template <typename T>
bool IntegralCompressor<T>::CompressDelta(T value1, T value2, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer)
{
    return CompressDeltaImpl(static_cast<ActualType>(value1), static_cast<ActualType>(value2), scheme, writer);
}

template <typename T>
void IntegralCompressor<T>::CompressFull(T value, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer)
{
    CompressFullImpl(static_cast<ActualType>(value), scheme, writer);
}

template <typename T>
void IntegralCompressor<T>::DecompressDelta(T baseValue, T* targetValue, CompressionScheme scheme, BitReader& reader)
{
    *targetValue = static_cast<T>(DecompressDeltaImpl(static_cast<ActualType>(baseValue), scheme, reader));
}

template <typename T>
void IntegralCompressor<T>::DecompressFull(T* targetValue, CompressionScheme scheme, BitReader& reader)
{
    *targetValue = static_cast<T>(DecompressFullImpl<ActualType>(scheme, reader));
}

//////////////////////////////////////////////////////////////////////////
template <typename T>
template <typename W>
W IntegralCompressor<T>::Abs(W v, select_signed<W>)
{
    return std::abs(v);
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::Abs(W v, select_unsigned<W>)
{
    using SW = std::make_signed_t<W>;
    return static_cast<W>(std::abs(static_cast<SW>(v)));
}

//////////////////////////////////////////////////////////////////////////
template <typename T>
template <typename W>
CompressionScheme IntegralCompressor<T>::GetCompressionSchemeImpl(const ReflectedMeta* meta, select_bool<W>)
{
    return 0;
}

template <typename T>
template <typename W>
CompressionScheme IntegralCompressor<T>::GetCompressionSchemeImpl(const ReflectedMeta* meta, select_lt32bit<W>)
{
    DVASSERT(CompressionUtils::GetIntCompressionSchemeFromMeta(meta) == 0 ||
             CompressionUtils::GetFullRangeRecord(CompressionUtils::GetIntCompressionSchemeFromMeta(meta))->bits <= sizeof(W) * 8);
    return CompressionUtils::GetIntCompressionSchemeFromMeta(meta);
}

template <typename T>
template <typename W>
CompressionScheme IntegralCompressor<T>::GetCompressionSchemeImpl(const ReflectedMeta* meta, select_64bit<W>)
{
    DVASSERT(CompressionUtils::GetIntCompressionSchemeFromMeta(meta) == 0 ||
             CompressionUtils::GetFullRangeRecord(CompressionUtils::GetIntCompressionSchemeFromMeta(meta))->bits <= sizeof(W) * 8);
    return CompressionUtils::GetInt64CompressionSchemeFromMeta(meta);
}

////  bool  //////////////////////////////////////////////////////////////
template <typename T>
template <typename W>
bool IntegralCompressor<T>::CompressDeltaImpl(W value1, W value2, CompressionScheme /*scheme*/, BitWriter& writer, select_bool<W>)
{
    if (value1 != value2)
    {
        writer.WriteBits(value2, 1);
        return true;
    }
    return false;
}

template <typename T>
template <typename W>
void IntegralCompressor<T>::CompressFullImpl(W value, CompressionScheme /*scheme*/, BitWriter& writer, select_bool<W>)
{
    writer.WriteBits(value, 1);
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressDeltaImpl(W /*baseValue*/, CompressionScheme /*scheme*/, BitReader& reader, select_bool<W>)
{
    return reader.ReadBits(1) != 0;
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressFullImpl(CompressionScheme /*scheme*/, BitReader& reader, select_bool<W>)
{
    return reader.ReadBits(1) != 0;
}

////  integer 8, 16 and 32 bits  /////////////////////////////////////////
template <typename T>
template <typename W>
bool IntegralCompressor<T>::CompressDeltaImpl(W value1, W value2, CompressionScheme scheme, BitWriter& writer, select_lt32bit<W>)
{
    if (value2 != value1)
    {
        const W delta = value2 - value1;
        if (scheme != 0)
        {
            const IntCompressionRecord* record = CompressionUtils::GetDeltaRangeRecord(scheme);
            DVASSERT(static_cast<uint32>(Abs(delta)) <= record->range, Format("Delta value %u exceeds range [-%u, %u]", static_cast<uint32>(Abs(delta)), record->range, record->range).c_str());

            const W adjusted = delta + static_cast<W>(record->range);
            writer.WriteBits(adjusted, record->bits);
        }
        else
        {
            writer.WriteBits(delta, sizeof(W) * 8);
        }
        return true;
    }
    return false;
}

template <typename T>
template <typename W>
void IntegralCompressor<T>::CompressFullImpl(W value, CompressionScheme scheme, BitWriter& writer, select_lt32bit<W>)
{
    if (scheme != 0)
    {
        const IntCompressionRecord* record = CompressionUtils::GetFullRangeRecord(scheme);
        DVASSERT(static_cast<uint32>(Abs(value)) <= record->range, Format("Value %u exceeds range [-%u, %u]", static_cast<uint32>(Abs(value)), record->range, record->range).c_str());

        const W adjusted = value + static_cast<W>(record->range);
        writer.WriteBits(adjusted, record->bits);
    }
    else
    {
        writer.WriteBits(value, sizeof(W) * 8);
    }
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressDeltaImpl(W baseValue, CompressionScheme scheme, BitReader& reader, select_lt32bit<W>)
{
    W v;
    if (scheme != 0)
    {
        const IntCompressionRecord* record = CompressionUtils::GetDeltaRangeRecord(scheme);
        v = static_cast<W>(reader.ReadBits(record->bits));
        v -= static_cast<W>(record->range);
    }
    else
    {
        v = static_cast<W>(reader.ReadBits(sizeof(W) * 8));
    }
    return baseValue + v;
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressFullImpl(CompressionScheme scheme, BitReader& reader, select_lt32bit<W>)
{
    W v;
    if (scheme != 0)
    {
        const IntCompressionRecord* record = CompressionUtils::GetFullRangeRecord(scheme);
        v = static_cast<W>(reader.ReadBits(record->bits));
        v -= static_cast<W>(record->range);
    }
    else
    {
        v = static_cast<W>(reader.ReadBits(sizeof(W) * 8));
    }
    return v;
}

////  integer 64 bits  ///////////////////////////////////////////////////
template <typename T>
template <typename W>
bool IntegralCompressor<T>::CompressDeltaImpl(W value1, W value2, CompressionScheme scheme, BitWriter& writer, select_64bit<W>)
{
    if (value2 != value1)
    {
        const W delta = value2 - value1;
        if (scheme != 0)
        {
            const Int64CompressionRecord* record = CompressionUtils::GetDeltaRange64Record(scheme);
            DVASSERT(static_cast<uint64>(Abs(delta)) <= record->range,
                     Format("Delta value %llu exceeds range [-%llu, %llu]", static_cast<unsigned long long>(Abs(delta)),
                            static_cast<unsigned long long>(record->range), static_cast<unsigned long long>(record->range))
                     .c_str());

            const W adjusted = delta + static_cast<W>(record->range);
            if (record->bits <= 32)
            {
                writer.WriteBits(static_cast<uint32>(adjusted), record->bits);
            }
            else
            {
                writer.WriteBits(static_cast<uint32>(adjusted), 32);
                writer.WriteBits(static_cast<uint32>(adjusted >> 32), record->bits - 32);
            }
        }
        else
        {
            writer.WriteBits(static_cast<uint32>(delta), 32);
            writer.WriteBits(static_cast<uint32>(delta >> 32), 32);
        }
        return true;
    }
    return false;
}

template <typename T>
template <typename W>
void IntegralCompressor<T>::CompressFullImpl(W value, CompressionScheme scheme, BitWriter& writer, select_64bit<W>)
{
    if (scheme != 0)
    {
        const Int64CompressionRecord* record = CompressionUtils::GetFullRange64Record(scheme);
        DVASSERT(static_cast<uint64>(Abs(value)) <= record->range,
                 Format("Value %llu exceeds range [-%llu, %llu]", static_cast<unsigned long long>(Abs(value)),
                        static_cast<unsigned long long>(record->range), static_cast<unsigned long long>(record->range))
                 .c_str());

        const W adjusted = value + static_cast<W>(record->range);
        if (record->bits <= 32)
        {
            writer.WriteBits(static_cast<uint32>(adjusted), record->bits);
        }
        else
        {
            writer.WriteBits(static_cast<uint32>(adjusted), 32);
            writer.WriteBits(static_cast<uint32>(adjusted >> 32), record->bits - 32);
        }
    }
    else
    {
        writer.WriteBits(static_cast<uint32>(value), 32);
        writer.WriteBits(static_cast<uint32>(value >> 32), 32);
    }
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressDeltaImpl(W baseValue, CompressionScheme scheme, BitReader& reader, select_64bit<W>)
{
    W v;
    if (scheme != 0)
    {
        const Int64CompressionRecord* record = CompressionUtils::GetDeltaRange64Record(scheme);
        if (record->bits <= 32)
        {
            v = static_cast<W>(reader.ReadBits(record->bits));
        }
        else
        {
            const uint64 lo = reader.ReadBits(32);
            const uint64 hi = reader.ReadBits(record->bits - 32);
            v = static_cast<W>(lo | (hi << 32));
        }
        v -= static_cast<W>(record->range);
    }
    else
    {
        const uint64 lo = reader.ReadBits(32);
        const uint64 hi = reader.ReadBits(32);
        v = static_cast<W>(lo | (hi << 32));
    }
    return baseValue + v;
}

template <typename T>
template <typename W>
W IntegralCompressor<T>::DecompressFullImpl(CompressionScheme scheme, BitReader& reader, select_64bit<W>)
{
    W v;
    if (scheme != 0)
    {
        const Int64CompressionRecord* record = CompressionUtils::GetFullRange64Record(scheme);
        if (record->bits <= 32)
        {
            v = static_cast<W>(reader.ReadBits(record->bits));
        }
        else
        {
            const uint64 lo = reader.ReadBits(32);
            const uint64 hi = reader.ReadBits(record->bits - 32);
            v = static_cast<W>(lo | (hi << 32));
        }
        v -= static_cast<W>(record->range);
    }
    else
    {
        const uint64 lo = reader.ReadBits(32);
        const uint64 hi = reader.ReadBits(32);
        v = static_cast<W>(lo | (hi << 32));
    }
    return v;
}

} // namespace DAVA
