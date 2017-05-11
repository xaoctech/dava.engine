#include "Tools/AssetCache/ChunkSplitter.h"

namespace DAVA
{
namespace AssetCache
{
namespace ChunkSplitter
{
const uint32 CHUNK_SIZE_IN_BYTES = 1 * 1024 * 1024;

uint32 GetNumberOfChunks(uint64 overallSize)
{
    uint32 res = static_cast<uint32>(overallSize / CHUNK_SIZE_IN_BYTES);
    if (overallSize % CHUNK_SIZE_IN_BYTES)
    {
        ++res;
    }
    return res;
}

Vector<uint8> GetChunk(const Vector<uint8>& dataVector, uint32 chunkNumber)
{
    uint64 firstByte = chunkNumber * CHUNK_SIZE_IN_BYTES;
    if (firstByte < dataVector.size())
    {
        uint64 beyondLastByte = std::min(static_cast<uint64>(dataVector.size()), firstByte + CHUNK_SIZE_IN_BYTES);
        return Vector<uint8>(dataVector.begin() + firstByte, dataVector.begin() + beyondLastByte);
    }
    else
    {
        return Vector<uint8>();
    }
}

}
} // namespace AssetCache
} // namespace DAVA
