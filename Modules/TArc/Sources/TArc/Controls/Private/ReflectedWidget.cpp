#include "TArc/Controls/ReflectedWidget.h"

namespace DAVA
{
ReflectedWidget::ReflectedWidget(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

ReflectedWidget::ReflectedWidget(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
}

void ReflectedWidget::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Visible))
    {
        bool value = GetFieldValue<bool>(Fields::Visible, isVisible());
        setVisible(value);
    }
}
} // namespace DAVA
