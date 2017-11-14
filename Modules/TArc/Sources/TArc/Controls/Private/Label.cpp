#include "TArc/Controls/Label.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Qt/QtString.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
Label::Label(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

Label::Label(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
}

void Label::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Text))
    {
        QString stringValue = GetFieldValue<QString>(Fields::Text, QString());
        setText(stringValue);
        setToolTip(stringValue);
    }
}

} // namespace TArc
} // namespace DAVA
