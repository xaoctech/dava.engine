#include "TArc/Controls/SpinSlider.h"
#include "TArc/Controls/Slider.h"
#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/QtBoxLayouts.h"

namespace DAVA
{
SpinSlider::SpinSlider(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl(params, model);
}

SpinSlider::SpinSlider(const Params& params, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), model, parent)
{
    SetupControl(params, model);
}

SpinSlider::~SpinSlider()
{
    TearDown();
}

void SpinSlider::ForceUpdate()
{
    DVASSERT(spin != nullptr);
    DVASSERT(slider != nullptr);
    spin->ForceUpdate();
    slider->ForceUpdate();
    TBase::ForceUpdate();
}

void SpinSlider::TearDown()
{
    if (spin == nullptr)
    {
        DVASSERT(slider == nullptr);
        return;
    }

    DVASSERT(spin != nullptr);
    DVASSERT(slider != nullptr);
    spin->TearDown();
    slider->TearDown();
    TBase::TearDown();

    spin = nullptr;
    slider = nullptr;
}

void SpinSlider::SetupControl(const Params& params, const Reflection& model)
{
    QtHBoxLayout* layout = new QtHBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(1);
    setLayout(layout);

    ControlDescriptor descr(params.fields);

    DoubleSpinBox::Params spinParams(params.accessor, params.ui, params.wndKey);
    spinParams.fields[DoubleSpinBox::Fields::Value] = descr.GetName(SpinSlider::Fields::SpinValue);
    spinParams.fields[DoubleSpinBox::Fields::Range] = descr.GetName(SpinSlider::Fields::SpinRange);
    spinParams.fields[DoubleSpinBox::Fields::IsEnabled] = descr.GetName(SpinSlider::Fields::Enabled);
    spin = new DoubleSpinBox(spinParams, params.accessor, model, this);

    Slider::Params sliderParams(params.accessor, params.ui, params.wndKey);
    sliderParams.fields[Slider::Fields::Value] = descr.GetName(SpinSlider::Fields::SliderValue);
    sliderParams.fields[Slider::Fields::Range] = descr.GetName(SpinSlider::Fields::SliderRange);
    sliderParams.fields[Slider::Fields::Enabled] = descr.GetName(SpinSlider::Fields::Enabled);
    sliderParams.fields[Slider::Fields::ImmediateValue] = descr.GetName(SpinSlider::Fields::SliderImmediateValue);

    slider = new Slider(sliderParams, model, this);

    layout->AddControl(spin);
    layout->AddControl(slider);
}

} // namespace DAVA
