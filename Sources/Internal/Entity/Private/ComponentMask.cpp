#include "Entity/ComponentMask.h"
#include "Entity/ComponentUtils.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
void ComponentMask::Set(const Type* type, bool value)
{
    uint32 componentId = ComponentUtils::GetRuntimeId(type);
    DVASSERT(componentId < bits.size());
    bits.set(componentId, value);
}

bool ComponentMask::IsSet(const Type* type) const
{
    uint32 componentId = ComponentUtils::GetRuntimeId(type);
    DVASSERT(componentId < bits.size());
    return bits.test(componentId);
}

} // namespace DAVA
