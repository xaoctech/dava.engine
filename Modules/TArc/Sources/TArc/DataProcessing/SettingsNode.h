#pragma once

#include "TArc/DataProcessing/DataNode.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
namespace Metas
{
struct SettingsSortKey
{
    SettingsSortKey(int32 sortKey_)
        : sortKey(sortKey_)
    {
    }

    int32 sortKey = 0;
};
} // namespace Metas
namespace M
{
// Greater value of sort key mean that settings will be upper in settings dialog
using SettingsSortKey = Meta<Metas::SettingsSortKey>;
} // namespace M

namespace TArc
{
class SettingsManager;
class PropertiesItem;

class SettingsNode : public DataNode
{
public:
    virtual ~SettingsNode() = default;
    virtual void Load(const PropertiesItem& settingsNode);
    virtual void Save(PropertiesItem& settingsNode) const;

protected:
    void ReflectedLoad(const PropertiesItem& settingsNode);
    void ReflectedSave(PropertiesItem& settingsNode) const;

private:
    DAVA_VIRTUAL_REFLECTION(SettingsNode, DataNode);
};
} // namespace TArc
} // namespace DAVA