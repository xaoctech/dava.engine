#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/BaseTypes.h>

#include <QSlider>

namespace DAVA
{
namespace TArc
{
class Slider : public ControlProxyImpl<QSlider>
{
    using TBase = ControlProxyImpl<QSlider>;

public:
    enum class Fields : uint32
    {
        IsEnabled,
        Range, // const DAVA::M::Range*
        Value, // int
        Orientation, // Qt::Orientation
        ImmediateValue, // Method<void(int)>
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    Slider(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    Slider(const Params& params, Reflection model, QWidget* parent = nullptr);

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& descriptor) override;
    void UpdateRange();

    void OnValuedChanged(int value);
    void OnSliderUp();

    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA