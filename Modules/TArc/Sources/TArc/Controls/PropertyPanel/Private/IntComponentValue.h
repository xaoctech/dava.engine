#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class IntComponentValue : public BaseComponentValue
{
public:
    IntComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    int32 GetValue() const;
    void SetValue(int32 v);

private:
    DAVA_VIRTUAL_REFLECTION(IntComponentValue, BaseComponentValue);
};
}
}