#include "TArc/Controls/PropertyPanel/Private/SliderComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/DoubleSpinBox.h"

#include <Base/Type.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

#include <QHBoxLayout>

namespace DAVA
{
namespace TArc
{
Any SliderComponentValue::GetMultipleValue() const
{
    return Any();
}

bool SliderComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (currentValue.IsEmpty())
    {
        return true;
    }

    return newValue != currentValue;
}

ControlProxy* SliderComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    Widget* container = new Widget(parent);
    container->SetLayout(new QHBoxLayout());

    const Type* t = nodes[0]->field.ref.GetValueType();
    bool isFloat = t->IsFloatingPoint();

    if (isFloat == true)
    {
        DoubleSpinBox::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[DoubleSpinBox::Fields::IsReadOnly] = BaseComponentValue::readOnlyFieldName;
        p.fields[DoubleSpinBox::Fields::ShowSpinArrows].BindConstValue(true);
        p.fields[DoubleSpinBox::Fields::Value] = "spinBoxValue";
        container->AddControl(new DoubleSpinBox(p, GetAccessor(), model, container->ToWidgetCast()));
    }
    else
    {
        IntSpinBox::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[IntSpinBox::Fields::IsReadOnly] = BaseComponentValue::readOnlyFieldName;
        p.fields[IntSpinBox::Fields::ShowSpinArrows].BindConstValue(true);
        p.fields[IntSpinBox::Fields::Value] = "spinBoxValue";
        container->AddControl(new IntSpinBox(p, GetAccessor(), model, container->ToWidgetCast()));
    }

    {
        Slider::Params p(GetAccessor(), GetUI(), GetWindowKey());
        p.fields[Slider::Fields::Enabled] = "sliderEnabled";
        p.fields[Slider::Fields::Orientation].BindConstValue(Qt::Horizontal);
        p.fields[Slider::Fields::Value] = "value";
        p.fields[Slider::Fields::ImmediateValue] = "immediateValue";
        p.fields[Slider::Fields::EditingState] = "sliderStateChanged";
        Slider* slider = new Slider(p, model, container->ToWidgetCast());
        container->AddControl(slider);

        container->ToWidgetCast()->setFocusProxy(slider->ToWidgetCast());
    }

    return container;
}

bool SliderComponentValue::IsSliderEnabled() const
{
    return !IsReadOnly();
}

Any SliderComponentValue::GetSliderValue() const
{
    if (state == Slider::Editing)
    {
        return cachedValue;
    }

    return GetValue();
}

void SliderComponentValue::SetSliderValue(const Any& v)
{
    if (state == Slider::Editing)
    {
        SetImmediateSliderValue(cachedValue);
    }
    SetValue(v);
}

Any SliderComponentValue::GetSpinBoxValue() const
{
    return GetValue();
}

void SliderComponentValue::SetSpinBoxValue(const Any& v)
{
    SetValue(v);
}

void SliderComponentValue::SetImmediateSliderValue(const Any& immV)
{
    Any currentValue = GetValue();
    if (IsValidValueToSet(immV, currentValue))
    {
        for (const std::shared_ptr<PropertyNode>& node : nodes)
        {
            node->cachedValue = immV;
            node->field.ref.SetValueWithCast(immV);
        }
    }
}

void SliderComponentValue::SetSliderState(Slider::State state_)
{
    state = state_;
    if (state == Slider::Editing)
    {
        cachedValue = GetValue();
    }
    else
    {
        cachedValue = Any();
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SliderComponentValue)
{
    ReflectionRegistrator<SliderComponentValue>::Begin(CreateComponentStructureWrapper<SliderComponentValue>())
    .Field("value", &SliderComponentValue::GetSliderValue, &SliderComponentValue::SetSliderValue)[M::ProxyMetaRequire()]
    .Field("spinBoxValue", &SliderComponentValue::GetSpinBoxValue, &SliderComponentValue::SetSpinBoxValue)[M::ProxyMetaRequire()]
    .Field("sliderEnabled", &SliderComponentValue::IsSliderEnabled, nullptr)
    .Method("immediateValue", &SliderComponentValue::SetImmediateSliderValue)
    .Method("sliderStateChanged", &SliderComponentValue::SetSliderState)
    .End();
}
} // namespace TArc
} // namespace DAVA
