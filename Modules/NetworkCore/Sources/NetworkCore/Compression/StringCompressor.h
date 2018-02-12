#pragma once

#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class FastName;
class ReflectedMeta;

class StringCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const String& value1, const String& value2, float32 /*comparePrecision*/);

    static bool CompressDelta(const String& value1, const String& value2, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);
    static void CompressFull(const String& value, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);

    static void DecompressDelta(const String& baseValue, String* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(String* targetValue, CompressionScheme scheme, BitReader& reader);
};

class FastNameCompressor
{
public:
    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(const FastName& value1, const FastName& value2, float32 /*comparePrecision*/);

    static bool CompressDelta(const FastName& value1, const FastName& value2, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);
    static void CompressFull(const FastName& value, CompressionScheme scheme, float32 /*deltaPrecision*/, BitWriter& writer);

    static void DecompressDelta(const FastName& baseValue, FastName* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(FastName* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
