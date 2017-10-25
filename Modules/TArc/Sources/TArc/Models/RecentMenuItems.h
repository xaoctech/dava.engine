#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/DataProcessing/Common.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/DataProcessing/PropertiesHolder.h"
#include "TArc/Qt/QtString.h"

#include "Functional/Function.h"
#include "Functional/Signal.h"

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"

#include <QList>

namespace DAVA
{
class KeyedArchive;
namespace TArc
{
class ContextAccessor;
} // namespace TArc
} // namespace DAVA

class RecentMenuItems
{
public:
    RecentMenuItems(RecentMenuItems& rmi) = delete;
    RecentMenuItems& operator=(RecentMenuItems& rmi) = delete;

    struct Params
    {
        Params(const DAVA::TArc::WindowKey& windowKey_, DAVA::TArc::ContextAccessor* accessor_, const DAVA::String& propertiesItemKey);

        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::TArc::UI* ui = nullptr;

        QString recentMenuName;
        DAVA::TArc::ActionPlacementInfo recentMenuPlacementInfo;

        QList<QString> menuSubPath;
        DAVA::Function<DAVA::uint32()> getMaximumCount;
        const DAVA::String propertiesItemKey;
        DAVA::TArc::FieldDescriptor predicateFieldDescriptor;
        DAVA::Function<DAVA::Any(const DAVA::Any&)> enablePredicate;
        DAVA::TArc::WindowKey windowKey;
    };

    RecentMenuItems(Params&& params);

    void Add(const DAVA::String& recent);
    DAVA::Signal<DAVA::String> actionTriggered;

    DAVA::Vector<DAVA::String> Get() const;

private:
    void InitMenuItems();
    void AddInternal(const DAVA::String& recent);
    void RemoveMenuItems();

    DAVA::TArc::QtConnections connections;

    Params params;
};
