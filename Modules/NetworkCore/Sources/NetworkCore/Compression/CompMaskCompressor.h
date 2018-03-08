#pragma once

#include "Entity/ComponentMask.h"
#include "NetworkCore/Compression/Compression.h"

namespace DAVA
{
class BitReader;
class BitWriter;
class ReflectedMeta;

class CompMaskCompressor
{
public:
    using MaskBitSet = ComponentMask::Bits;

    static CompressionScheme GetCompressionScheme(const ReflectedMeta* meta);
    static float32 GetDeltaPrecision(const ReflectedMeta* meta);
    static float32 GetComparePrecision(const ReflectedMeta* meta);

    static bool IsEqual(ComponentMask value1, ComponentMask value2, float32 comparePrecision);

    static bool CompressDelta(ComponentMask value1, ComponentMask value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);
    static void CompressFull(ComponentMask value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer);

    static void DecompressDelta(ComponentMask baseValue, ComponentMask* targetValue, CompressionScheme scheme, BitReader& reader);
    static void DecompressFull(ComponentMask* targetValue, CompressionScheme scheme, BitReader& reader);
};

} // namespace DAVA
