#pragma once

#include <Base/BaseTypes.h>
#include <Base/Type.h>

#include "NetworkCore/Compression/AnyCompressor.h"
#include "NetworkCore/Compression/FixedVectorCompressor.h"
#include "NetworkCore/Compression/IntegralCompressor.h"

namespace DAVA
{
struct CompressorInterface;

void RegisterStandardTypeCompressors();
void RegisterTypeCompressor(const Type* type, const CompressorInterface* compressor);

template <typename T, typename TCompressor>
void RegisterTypeCompressor()
{
    static const AnyCompressor<T, TCompressor> compressor{};
    static const AnyCompressor<FixedVector<T>, FixedVectorCompressor<T, TCompressor>> vectorCompressor{};

    const Type* type = Type::Instance<T>();
    const Type* vectorType = Type::Instance<FixedVector<T>>();
    RegisterTypeCompressor(type, &compressor);
    RegisterTypeCompressor(vectorType, &vectorCompressor);
}

} // namespace DAVA
