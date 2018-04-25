#include "Entity/SystemProcessInfo.h"

namespace DAVA
{
SystemProcessInfo::SystemProcessInfo(SPI::Group group, SPI::Type type, float32 order)
    : group(group)
    , type(type)
    , order(order)
{
}

bool SystemProcessInfo::operator==(const SystemProcessInfo& other) const
{
    return group == other.group && type == other.type && order == other.order;
}

bool SystemProcessInfo::operator!=(const SystemProcessInfo& other) const
{
    return !(*this == other);
}

bool SystemProcessInfo::operator<(const SystemProcessInfo& other) const
{
    return group == other.group ? order < other.order : group < other.group;
}
} // namespace DAVA