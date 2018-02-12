#pragma once

#include <Base/BaseTypes.h>
#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Base/FastName.h>

//#define COMPRESSOR_DEBUG_ENABLED
//#define COMPRESSION_DISABLED

namespace DAVA
{
using CompressionScheme = uint32;

class Any;
class BitReader;
class BitWriter;
class Quaternion;
class ReflectedMeta;
class Type;
struct CompressorInterface;

namespace Metas
{
struct FloatQuantizeParam;
struct QuaternionQuantizeParam;
struct IntCompressParam;
struct Int64CompressParam;
}

struct FracCompressionRecord
{
    uint32 bits;
    float32 precision;
};

struct IntCompressionRecord
{
    uint32 bits;
    uint32 range;
};

struct Int64CompressionRecord
{
    uint32 bits;
    uint64 range;
};

const uint32 maxArraySize = 254;

const float32 defaultComparePrecision = 0.00001f;
const float32 defaultDeltaPrecision = 0.000001f;

struct CompressorInterface
{
    virtual FastName GetTypeName() const = 0;

    virtual CompressionScheme GetCompressionScheme(const ReflectedMeta* meta) const = 0;
    virtual float32 GetDeltaPrecision(const ReflectedMeta* meta) const = 0;
    virtual float32 GetComparePrecision(const ReflectedMeta* meta) const = 0;

    virtual bool IsEqual(const Any& any1, const Any& any2, float32 comparePrecision) const = 0;

    virtual bool CompressDelta(const Any& any1, const Any& any2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const = 0;
    virtual void CompressFull(const Any& any, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const = 0;

    virtual void DecompressDelta(const Any& anyBase, Any& anyTarget, CompressionScheme scheme, BitReader& reader) const = 0;
    virtual void DecompressFull(Any& anyTarget, CompressionScheme scheme, BitReader& reader) const = 0;
};

struct CompressionUtils
{
    static CompressionScheme MakeCompressionScheme(const Metas::FloatQuantizeParam* param);
    static CompressionScheme MakeCompressionScheme(const Metas::QuaternionQuantizeParam* param);
    static CompressionScheme MakeCompressionScheme(const Metas::IntCompressParam* param);
    static CompressionScheme MakeCompressionScheme(const Metas::Int64CompressParam* param);

    static CompressionScheme GetFloatCompressionSchemeFromMeta(const ReflectedMeta* meta);
    static CompressionScheme GetQuaternionCompressionSchemeFromMeta(const ReflectedMeta* meta);
    static CompressionScheme GetIntCompressionSchemeFromMeta(const ReflectedMeta* meta);
    static CompressionScheme GetInt64CompressionSchemeFromMeta(const ReflectedMeta* meta);
    static float32 GetComparePrecisionFromMeta(const ReflectedMeta* meta, float32 comparePrecision = defaultComparePrecision);

    static const FracCompressionRecord* GetFracRecord(CompressionScheme scheme);
    static const FracCompressionRecord* GetQuaternionRecord(CompressionScheme scheme);
    static const IntCompressionRecord* GetFullRangeRecord(CompressionScheme scheme);
    static const IntCompressionRecord* GetDeltaRangeRecord(CompressionScheme scheme);
    static const Int64CompressionRecord* GetFullRange64Record(CompressionScheme scheme);
    static const Int64CompressionRecord* GetDeltaRange64Record(CompressionScheme scheme);

    static float32 GetFloatDeltaPrecision(CompressionScheme scheme, float32 deltaPrecision = defaultDeltaPrecision);
    static float32 GetQuaternionDeltaPrecision(CompressionScheme scheme, float32 deltaPrecision = defaultDeltaPrecision);

    static void CompressFloat(float32 value, float32 deltaPrecision, const IntCompressionRecord* intRecord, const FracCompressionRecord* fracRecord, BitWriter& writer);
    static float32 DecompressFloat(const IntCompressionRecord* intRecord, const FracCompressionRecord* fracRecord, BitReader& reader);

    static void CompressQuaternion(const Quaternion& q, const FracCompressionRecord* qRecord, BitWriter& writer);
    static Quaternion DecompressQuaternion(const FracCompressionRecord* qRecord, BitReader& reader);

    template <typename T>
    static uint32 CompressVarInt(T value, BitWriter& writer);
    template <typename T>
    static T DecompressVarInt(BitReader& reader);

    static const CompressorInterface* GetTypeCompressor(const Type* type);
    static const CompressorInterface* GetTypeCompressor(const Any& any);

private:
    template <typename T>
    using EnableIfOneByte = std::enable_if_t<sizeof(T) == 1, int>;
    template <typename T>
    using EnableIfUnsigned = std::enable_if_t<sizeof(T) != 1 && std::is_unsigned<T>::value, int>;
    template <typename T>
    using EnableIfSigned = std::enable_if_t<sizeof(T) != 1 && std::is_signed<T>::value, int>;

    template <typename T, EnableIfOneByte<T> = 0>
    static void CompressVarIntImpl(T value, BitWriter& writer);
    template <typename T, EnableIfUnsigned<T> = 0>
    static void CompressVarIntImpl(T value, BitWriter& writer);
    template <typename T, EnableIfSigned<T> = 0>
    static void CompressVarIntImpl(T value, BitWriter& writer);

    template <typename T, EnableIfOneByte<T> = 0>
    static T DecompressVarIntImpl(BitReader& reader);
    template <typename T, EnableIfUnsigned<T> = 0>
    static T DecompressVarIntImpl(BitReader& reader);
    template <typename T, EnableIfSigned<T> = 0>
    static T DecompressVarIntImpl(BitReader& reader);
};

template <typename T>
uint32 CompressionUtils::CompressVarInt(T value, BitWriter& writer)
{
    uint32 x = writer.GetBitsWritten();
    CompressVarIntImpl(value, writer);
    return writer.GetBitsWritten() - x;
}

template <typename T>
T CompressionUtils::DecompressVarInt(BitReader& reader)
{
    return DecompressVarIntImpl<T>(reader);
}

//////////////////////////////////////////////////////////////////////////
template <typename T, CompressionUtils::EnableIfOneByte<T>>
void CompressionUtils::CompressVarIntImpl(T value, BitWriter& writer)
{
    writer.WriteBits(value, 8);
}

template <typename T, CompressionUtils::EnableIfUnsigned<T>>
void CompressionUtils::CompressVarIntImpl(T value, BitWriter& writer)
{
    while (value > 127)
    {
        uint8 x = static_cast<uint8>(value);
        x |= 0x80;
        writer.WriteBits(x, 8);
        value >>= 7;
    }
    writer.WriteBits(static_cast<uint32>(value), 8);
}

template <typename T, CompressionUtils::EnableIfSigned<T>>
void CompressionUtils::CompressVarIntImpl(T value, BitWriter& writer)
{
    using unsignedT = std::make_unsigned_t<T>;
    const uint32 bitCount = sizeof(T) * 8;
    value = (value << 1) ^ (value >> (bitCount - 1));
    CompressVarIntImpl(static_cast<unsignedT>(value), writer);
}

//////////////////////////////////////////////////////////////////////////
template <typename T, CompressionUtils::EnableIfOneByte<T>>
T CompressionUtils::DecompressVarIntImpl(BitReader& reader)
{
    return static_cast<T>(reader.ReadBits(8));
}

template <typename T, CompressionUtils::EnableIfUnsigned<T>>
T CompressionUtils::DecompressVarIntImpl(BitReader& reader)
{
    T v = 0;
    int shift = 0;
    uint8 x = 0;
    do
    {
        x = reader.ReadBits(8);
        v |= (static_cast<T>(x & 0x7F) << shift);
        shift += 7;
    } while (x > 127);
    return v;
}
template <typename T, CompressionUtils::EnableIfSigned<T>>
T CompressionUtils::DecompressVarIntImpl(BitReader& reader)
{
    using unsignedT = std::make_unsigned_t<T>;
    T v = static_cast<T>(DecompressVarIntImpl<unsignedT>(reader));
    v = (v >> 1) ^ (-(v & 1));
    return v;
}

} // namespace DAVA
