#pragma once

#include "NetworkCore/Compression/Compression.h"
#include "NetworkCore/Compression/ArrayCompressor.h"

#include <Base/Any.h>
#include <Base/Type.h>
#include <Debug/Backtrace.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
template <typename T, typename TCompressor>
struct AnyCompressor final : public CompressorInterface
{
    using ArrayCompressor = ArrayCompressor<T, TCompressor>;

    FastName GetTypeName() const final;

    CompressionScheme GetCompressionScheme(const ReflectedMeta* meta) const final;
    float32 GetDeltaPrecision(const ReflectedMeta* meta) const final;
    float32 GetComparePrecision(const ReflectedMeta* meta) const final;

    bool IsEqual(const Any& any1, const Any& any2, float32 comparePrecision) const final;

    bool CompressDelta(const Any& any1, const Any& any2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const final;
    void CompressFull(const Any& any, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const final;

    void DecompressDelta(const Any& anyBase, Any& anyTarget, CompressionScheme scheme, BitReader& reader) const final;
    void DecompressFull(Any& anyTarget, CompressionScheme scheme, BitReader& reader) const final;
};

template <typename T, typename TCompressor>
FastName AnyCompressor<T, TCompressor>::GetTypeName() const
{
    static FastName name(Debug::DemangleFrameSymbol(Type::Instance<T>()->GetName()));
    return name;
}

template <typename T, typename TCompressor>
float32 AnyCompressor<T, TCompressor>::GetDeltaPrecision(const ReflectedMeta* meta) const
{
    return TCompressor::GetDeltaPrecision(meta);
}

template <typename T, typename TCompressor>
CompressionScheme AnyCompressor<T, TCompressor>::GetCompressionScheme(const ReflectedMeta* meta) const
{
    return TCompressor::GetCompressionScheme(meta);
}

template <typename T, typename TCompressor>
float32 AnyCompressor<T, TCompressor>::GetComparePrecision(const ReflectedMeta* meta) const
{
    return TCompressor::GetComparePrecision(meta);
}

template <typename T, typename TCompressor>
bool AnyCompressor<T, TCompressor>::IsEqual(const Any& any1, const Any& any2, float32 comparePrecision) const
{
    DVASSERT(any1.GetType() == any2.GetType());

    const Type* type = any1.GetType();
    if (!type->IsArray())
    {
        const T& value1 = any1.Get<T>();
        const T& value2 = any2.Get<T>();
        return TCompressor::IsEqual(value1, value2, comparePrecision);
    }
    else
    {
        DVASSERT(any1.GetType()->GetArrayDimension() == any2.GetType()->GetArrayDimension());
        DVASSERT(any1.GetType()->GetArrayElementType() == any2.GetType()->GetArrayElementType());

        const size_t size = type->GetArrayDimension();
        const T* seq1 = static_cast<const T*>(any1.GetData());
        const T* seq2 = static_cast<const T*>(any2.GetData());
        return ArrayCompressor::IsEqual(seq1, seq2, size, comparePrecision);
    }
}

template <typename T, typename TCompressor>
bool AnyCompressor<T, TCompressor>::CompressDelta(const Any& any1, const Any& any2, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const
{
    DVASSERT(any1.GetType() == any2.GetType());

    const Type* type = any1.GetType();
    if (!type->IsArray())
    {
        const T& value1 = any1.Get<T>();
        const T& value2 = any2.Get<T>();
        return TCompressor::CompressDelta(value1, value2, scheme, deltaPrecision, writer);
    }
    else
    {
        DVASSERT(any1.GetType()->GetArrayDimension() == any2.GetType()->GetArrayDimension());
        DVASSERT(any1.GetType()->GetArrayElementType() == any2.GetType()->GetArrayElementType());

        const size_t size = type->GetArrayDimension();
        const T* seq1 = static_cast<const T*>(any1.GetData());
        const T* seq2 = static_cast<const T*>(any2.GetData());
        return ArrayCompressor::CompressDelta(seq1, seq2, size, scheme, deltaPrecision, writer);
    }
}

template <typename T, typename TCompressor>
void AnyCompressor<T, TCompressor>::CompressFull(const Any& any, CompressionScheme scheme, float32 deltaPrecision, BitWriter& writer) const
{
    const Type* type = any.GetType();
    if (!type->IsArray())
    {
        const T& value = any.Get<T>();
        TCompressor::CompressFull(value, scheme, deltaPrecision, writer);
    }
    else
    {
        const size_t size = type->GetArrayDimension();
        const T* seq = static_cast<const T*>(any.GetData());
        ArrayCompressor::CompressFull(seq, size, scheme, deltaPrecision, writer);
    }
}

template <typename T, typename TCompressor>
void AnyCompressor<T, TCompressor>::DecompressDelta(const Any& anyBase, Any& anyTarget, CompressionScheme scheme, BitReader& reader) const
{
    DVASSERT(anyBase.GetType() == anyTarget.GetType());

    const Type* type = anyBase.GetType();
    if (!type->IsArray())
    {
        const T& base = anyBase.Get<T>();
        T target = base;
        TCompressor::DecompressDelta(base, &target, scheme, reader);
        anyTarget.Set(target);
    }
    else
    {
        DVASSERT(anyBase.GetType()->GetArrayDimension() == anyTarget.GetType()->GetArrayDimension());
        DVASSERT(anyBase.GetType()->GetArrayElementType() == anyTarget.GetType()->GetArrayElementType());

        const size_t size = type->GetArrayDimension();
        const T* baseSeq = static_cast<const T*>(anyBase.GetData());

        Vector<uint8> buf(size * sizeof(T), 0);
        T* targetSeq = reinterpret_cast<T*>(buf.data());
        std::memcpy(targetSeq, baseSeq, size * sizeof(T));
        ArrayCompressor::DecompressDelta(baseSeq, targetSeq, size, scheme, reader);

        anyTarget.SetTrivially(targetSeq, type);
    }
}

template <typename T, typename TCompressor>
void AnyCompressor<T, TCompressor>::DecompressFull(Any& anyTarget, CompressionScheme scheme, BitReader& reader) const
{
    const Type* type = anyTarget.GetType();
    if (!type->IsArray())
    {
        T target = anyTarget.Get<T>();
        TCompressor::DecompressFull(&target, scheme, reader);
        anyTarget.Set(target);
    }
    else
    {
        const size_t size = type->GetArrayDimension();
        Vector<uint8> buf(size * sizeof(T), 0);
        T* targetSeq = reinterpret_cast<T*>(buf.data());
        ArrayCompressor::DecompressFull(targetSeq, size, scheme, reader);

        anyTarget.SetTrivially(targetSeq, type);
    }
}

} // namespace DAVA
