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

bool AnyFn::Params::IsMatching(const AnyFn::Params& params) const
{
    if (retType != params.retType)
    {
        return false;
    }

    size_t sz = argsType.size();

    if (sz != params.argsType.size())
    {
        return false;
    }

    for (size_t i = 0; i < sz; ++i)
    {
        if ((argsType[i] != params.argsType[i]) && (argsType[i]->Decay() != params.argsType[i]))
            return false;
    }
    return true;
}
} // namespace DAVA
