#pragma once

#include "TArc/Controls/ControlProxy.h"

namespace DAVA
{
class SpinSlider : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    enum class Fields
    {
        Enabled,
        SliderRange, // const DAVA::M::Range*
        SliderValue, // int or float
        SliderImmediateValue, // Method<void(int)>
        SpinRange, // const DAVA::M::Range*
        SpinValue, // float32
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    SpinSlider(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    SpinSlider(const Params& params, Reflection model, QWidget* parent = nullptr);
    ~SpinSlider();

    void ForceUpdate() override;
    void TearDown() override;

protected:
    void SetupControl(const Params& params, const Reflection& model);
    void UpdateControl(const ControlDescriptor& descriptor) override
    {
    }

    ControlProxy* spin = nullptr;
    ControlProxy* slider = nullptr;
};
} // namespace DAVA
