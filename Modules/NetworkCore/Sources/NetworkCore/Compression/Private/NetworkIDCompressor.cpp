#include "NetworkCore/Compression/NetworkIDCompressor.h"
#include "NetworkCore/Compression/IntegralCompressor.h"

#include <Base/BitReader.h>
#include <Base/BitWriter.h>

namespace DAVA
{
CompressionScheme NetworkIDCompressor::GetCompressionScheme(const ReflectedMeta* meta)
{
    return IntegralCompressor<uint32>::GetCompressionScheme(meta);
}

float32 NetworkIDCompressor::GetDeltaPrecision(const ReflectedMeta* meta)
{
    return IntegralCompressor<uint32>::GetDeltaPrecision(meta);
}

float32 NetworkIDCompressor::GetComparePrecision(const ReflectedMeta* meta)
{
    return IntegralCompressor<uint32>::GetComparePrecision(meta);
}

bool NetworkIDCompressor::IsEqual(NetworkID value1, NetworkID value2, float32 comparePrecision)
{
    return IntegralCompressor<uint32>::IsEqual(static_cast<uint32>(value1), static_cast<uint32>(value2), comparePrecision);
}

bool NetworkIDCompressor::CompressDelta(NetworkID value1, NetworkID value2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    return IntegralCompressor<uint32>::CompressDelta(static_cast<uint32>(value1), static_cast<uint32>(value2), scheme, deltaPrecision, writer);
}

void NetworkIDCompressor::CompressFull(NetworkID value, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer)
{
    IntegralCompressor<uint32>::CompressFull(static_cast<uint32>(value), scheme, deltaPrecision, writer);
}

void NetworkIDCompressor::DecompressDelta(NetworkID baseValue, NetworkID* targetValue, CompressionScheme scheme, BitReader& reader)
{
    uint32 rawNetworkID;
    IntegralCompressor<uint32>::DecompressDelta(static_cast<uint32>(baseValue), &rawNetworkID, scheme, reader);
    *targetValue = NetworkID(rawNetworkID);
}

void NetworkIDCompressor::DecompressFull(NetworkID* targetValue, CompressionScheme scheme, BitReader& reader)
{
    uint32 rawNetworkID;
    IntegralCompressor<uint32>::DecompressFull(&rawNetworkID, scheme, reader);
    *targetValue = NetworkID(rawNetworkID);
}

} // namespace DAVA
