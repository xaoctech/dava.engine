#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/StringCompressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>
#include <Base/FastName.h>
#include <Base/String.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
CompressionScheme StringCompressor::GetCompressionScheme(const ReflectedMeta* /*meta*/)
{
    return 0;
}

float32 StringCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

float32 StringCompressor::GetComparePrecision(const ReflectedMeta* /*meta*/)
{
    return 0.f;
}

bool StringCompressor::IsEqual(const String& value1, const String& value2, float32 /*comparePrecision*/)
{
    return value1 == value2;
}

bool StringCompressor::CompressDelta(const String& value1, const String& value2, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    if (value1 != value2)
    {
        CompressFull(value2, 0, 0.f, writer);
        return true;
    }
    return false;
}

void StringCompressor::CompressFull(const String& value, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    uint32 length = static_cast<uint32>(value.length());
    DVASSERT(length < 250);
    writer.WriteBits(length, 8);
    for (char c : value)
    {
        writer.WriteBits(c, 8);
    }
}

void StringCompressor::DecompressDelta(const String& baseValue, String* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    DecompressFull(targetValue, 0, reader);
}

void StringCompressor::DecompressFull(String* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    String r;
    uint32 length = reader.ReadBits(8);
    r.reserve(length);
    for (uint32 i = 0; i < length; ++i)
    {
        char c = static_cast<char>(reader.ReadBits(8));
        r.push_back(c);
    }
    *targetValue = std::move(r);
}

CompressionScheme FastNameCompressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return 0;
}

float32 FastNameCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

float32 FastNameCompressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return 0.f;
}

bool FastNameCompressor::IsEqual(const FastName& value1, const FastName& value2, float32 /*comparePrecision*/)
{
    return value1 == value2;
}

bool FastNameCompressor::CompressDelta(const FastName& value1, const FastName& value2, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    if (value1 != value2)
    {
        CompressFull(value2, 0, 0.f, writer);
        return true;
    }
    return false;
}

void FastNameCompressor::CompressFull(const FastName& value, CompressionScheme /*scheme*/, float32 /*deltaPrecision*/, BitWriter& writer)
{
    uint32 length = static_cast<uint32>(value.size());
    const char* str = value.c_str();
    DVASSERT(length < 250);
    writer.WriteBits(length, 8);
    for (uint32 i = 0; i < length; ++i)
    {
        writer.WriteBits(str[i], 8);
    }
}

void FastNameCompressor::DecompressDelta(const FastName& baseValue, FastName* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    DecompressFull(targetValue, 0, reader);
}

void FastNameCompressor::DecompressFull(FastName* targetValue, CompressionScheme /*scheme*/, BitReader& reader)
{
    String r;
    uint32 length = reader.ReadBits(8);
    r.reserve(length);
    for (uint32 i = 0; i < length; ++i)
    {
        char c = static_cast<char>(reader.ReadBits(8));
        r.push_back(c);
    }
    *targetValue = FastName(r);
}

} // namespace DAVA
