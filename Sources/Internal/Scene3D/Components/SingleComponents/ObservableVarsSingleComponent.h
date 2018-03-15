#pragma once

#include "Base/Any.h"
#include "Base/UnordererMap.h"
#include "Entity/SingleComponent.h"

namespace DAVA
{
class IVar;

class ObservableVarsSingleComponent : public SingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(ObservableVarsSingleComponent, SingleComponent);

    void NotifyAboutChanges(const IVar* var, Any&& value);
    const UnorderedMap<const IVar*, Any>& GetChanges() const;
    void Clear();

private:
    UnorderedMap<const IVar*, Any> changes;
};
}
