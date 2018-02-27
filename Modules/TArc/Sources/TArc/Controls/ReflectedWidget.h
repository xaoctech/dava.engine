#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QWidget>

namespace DAVA
{
class ReflectedWidget final : public ControlProxyImpl<QWidget>
{
public:
    enum class Fields : uint32
    {
        Visible,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ReflectedWidget(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ReflectedWidget(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
};
} // namespace DAVA
