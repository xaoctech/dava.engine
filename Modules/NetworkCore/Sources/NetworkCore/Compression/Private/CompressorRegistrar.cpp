#include "NetworkCore/Compression/CompressorRegistrar.h"

#include <Base/FastName.h>
#include <Base/FixedVector.h>
#include <Debug/DVAssert.h>
#include <Math/Matrix4.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/FloatCompressor.h"
#include "NetworkCore/Compression/IntegralCompressor.h"
#include "NetworkCore/Compression/Matrix4Compressor.h"
#include "NetworkCore/Compression/NetworkIDCompressor.h"
#include "NetworkCore/Compression/QuaternionCompressor.h"
#include "NetworkCore/Compression/StringCompressor.h"
#include "NetworkCore/Compression/Vector2Compressor.h"
#include "NetworkCore/Compression/Vector3Compressor.h"
#include "NetworkCore/Compression/BitsetCompressor.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

namespace DAVA
{
void RegisterStandardTypeCompressors()
{
    RegisterTypeCompressor<bool, IntegralCompressor<bool>>();
    RegisterTypeCompressor<int8, IntegralCompressor<int8>>();
    RegisterTypeCompressor<uint8, IntegralCompressor<uint8>>();
    RegisterTypeCompressor<int16, IntegralCompressor<int16>>();
    RegisterTypeCompressor<uint16, IntegralCompressor<uint16>>();
    RegisterTypeCompressor<int32, IntegralCompressor<int32>>();
    RegisterTypeCompressor<uint32, IntegralCompressor<uint32>>();
    RegisterTypeCompressor<int64, IntegralCompressor<int64>>();
    RegisterTypeCompressor<uint64, IntegralCompressor<uint64>>();

    RegisterTypeCompressor<float32, FloatCompressor>();

    RegisterTypeCompressor<Matrix4, Matrix4Compressor>();
    RegisterTypeCompressor<Quaternion, QuaternionCompressor>();
    RegisterTypeCompressor<Vector2, Vector2Compressor>();
    RegisterTypeCompressor<Vector3, Vector3Compressor>();

    RegisterTypeCompressor<FastName, FastNameCompressor>();
    RegisterTypeCompressor<String, StringCompressor>();

    RegisterTypeCompressor<NetworkID, NetworkIDCompressor>();
    RegisterTypeCompressor<ComponentMask, BitsetCompressor<ComponentMask>>();
}

uint32 GetTypeCompressorIndex()
{
    const uint32 nonAllocatedIndex = uint32(-1);
    static uint32 index = nonAllocatedIndex;
    if (index == nonAllocatedIndex)
    {
        index = Type::AllocUserData();
    }
    return index;
}

void RegisterTypeCompressor(const Type* type, const CompressorInterface* compressor)
{
    DVASSERT(type != nullptr && compressor != nullptr);

    const uint32 index = GetTypeCompressorIndex();
    void* prevData = type->GetUserData(index);
    DVASSERT(prevData == nullptr || prevData == compressor);

    if (prevData == nullptr)
    {
        type->SetUserData(index, const_cast<void*>(static_cast<const void*>(compressor)));
    }
}

} // namespace DAVA
