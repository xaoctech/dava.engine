#pragma once

#include "Base/Type.h"
#include "Base/Any.h"
#include "Base/FastName.h"

namespace std
{
template <>
struct hash<DAVA::Any>
{
    std::size_t operator()(const DAVA::Any& any) const
    {
        // think about more smart way how to calculate the hashing function
        const DAVA::Type* type = any.GetType();
        if (type->IsPointer())
        {
            void* ptr = any.Get<void*>();
            return reinterpret_cast<std::size_t>(&any);
        }
        else if (type->IsIntegral())
        {
            DAVA::int32 val = any.Cast<DAVA::int32>();
            return val;
        }
        else
        {
            if (any.CanGet<DAVA::FastName>())
            {
                const DAVA::FastName& fastName = any.Get<DAVA::FastName>();
                //std::size_t res = std::hash<const DAVA::FastName>(fastName);
                return reinterpret_cast<size_t>(fastName.c_str());
            }
            else if (any.CanGet<DAVA::String>())
            {
                const DAVA::String& fastName = any.Get<DAVA::String>();
                //return std::hash<DAVA::String>(fastName);
            }
        }
        DVASSERT(0 && "Failed to calculate the hash to any");
        return 0;
    }
};
}
