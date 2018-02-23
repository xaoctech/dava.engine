#include "ReflectionDeclaration/Private/AnyHashRegistration.h"

#include "Base/Any.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace AnyHashRegistrationDetail
{
template <typename T>
size_t DefaultHash(const Any& v)
{
    std::hash<T> hashFn;
    return hashFn(v.Get<T>());
}
} // namespace AnyHashRegistrationDetail

void RegisterAnyHash()
{
    AnyHash<uint8>::Register(&AnyHashRegistrationDetail::DefaultHash<uint8>);
    AnyHash<int8>::Register(&AnyHashRegistrationDetail::DefaultHash<int8>);
    AnyHash<uint16>::Register(&AnyHashRegistrationDetail::DefaultHash<uint16>);
    AnyHash<int16>::Register(&AnyHashRegistrationDetail::DefaultHash<int16>);
    AnyHash<uint32>::Register(&AnyHashRegistrationDetail::DefaultHash<uint32>);
    AnyHash<int32>::Register(&AnyHashRegistrationDetail::DefaultHash<int32>);
    AnyHash<uint64>::Register(&AnyHashRegistrationDetail::DefaultHash<uint64>);
    AnyHash<int64>::Register(&AnyHashRegistrationDetail::DefaultHash<int64>);
    AnyHash<char8>::Register(&AnyHashRegistrationDetail::DefaultHash<char8>);
    AnyHash<char16>::Register(&AnyHashRegistrationDetail::DefaultHash<char16>);
    AnyHash<float32>::Register(&AnyHashRegistrationDetail::DefaultHash<float32>);
    AnyHash<float64>::Register(&AnyHashRegistrationDetail::DefaultHash<float64>);
    AnyHash<bool>::Register(&AnyHashRegistrationDetail::DefaultHash<bool>);
    AnyHash<pointer_size>::Register(&AnyHashRegistrationDetail::DefaultHash<pointer_size>);
    AnyHash<size_t>::Register(&AnyHashRegistrationDetail::DefaultHash<size_t>);
}
} // namespace DAVA
