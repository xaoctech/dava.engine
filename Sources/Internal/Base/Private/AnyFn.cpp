#include "Base/AnyFn.h"

namespace DAVA
{
bool AnyFn::Params::operator==(const AnyFn::Params& p) const
{
    if (retType != p.retType)
    {
        return false;
    }

    size_t sz = argsType.size();

    if (sz != p.argsType.size())
    {
        return false;
    }

    return (0 == std::memcmp(argsType.data(), p.argsType.data(), sizeof(void*) * sz));
}

bool AnyFn::CanBeCalledWithParams(const AnyFn::Params& params) const
{
    const AnyFn::Params& myParams = GetInvokeParams();
    if (myParams.retType != params.retType)
    {
        return false;
    }

    size_t sz = myParams.argsType.size();

    if (sz != params.argsType.size())
    {
        return false;
    }

    for (size_t i = 0; i < sz; ++i)
    {
        if ((myParams.argsType[i] != params.argsType[i]) && (myParams.argsType[i]->Decay() != params.argsType[i]))
            return false;
    }
    return true;
}

} // namespace DAVA
