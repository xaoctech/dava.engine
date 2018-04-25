#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace SPI // SPI stands for SystemProcessInfo.
{
/** System process type. */
enum class Type : uint8
{
    Normal = 0, //!< normal system process type. If system process should be executed once per frame, choose this group.
    Fixed, //!< fixed system process type. If system process should be executed once per fixed update time, choose this group.
    Input //!< input system process type. If system process should handle input events, choose this group.
};

/** System process group. */
enum class Group : uint8
{
    EngineBegin = 0, //!< first part of base engine systems processes/fixed processes. Do not use this group for gameplay processes/fixed processes.
    Gameplay, //!< gameplay systems processes/fixed processes.
    EngineEnd //!< last part of base engine systems processes/fixed processes. Do not use this group for gameplay processes/fixed processes.
};
} // namespace SPI

/**
    Describes info for system process.
*/
class SystemProcessInfo
{
public:
    SystemProcessInfo(SPI::Group group, SPI::Type type, float32 order);

    bool operator==(const SystemProcessInfo& other) const;

    bool operator!=(const SystemProcessInfo& other) const;

    bool operator<(const SystemProcessInfo& other) const;

    SPI::Group group;
    SPI::Type type;
    float32 order;
};
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::SystemProcessInfo>
{
    size_t operator()(const DAVA::SystemProcessInfo& info) const
    {
        using DAVA::float32;
        // +.1f to avoid `0` on both sides of `/`.
        return std::hash<float32>()((static_cast<float32>(info.group) + .1f) / (static_cast<float32>(info.type) + .1f)) ^ std::hash<DAVA::float32>()(info.order);
    }
};
} // namespace std