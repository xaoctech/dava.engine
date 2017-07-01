#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"

#include <QComboBox>

//comboBox can be in invalid state so give it DAVA::Any as value

class ScaleComboBox final : public DAVA::TArc::ControlProxyImpl<QComboBox>
{
public:
    enum class Fields : DAVA::uint32
    {
        Value,
        Enumerator,
        Enabled,
        FieldCount
    };

    struct Params : BaseParams
    {
        Params(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, const DAVA::TArc::WindowKey& wndKey)
            : BaseParams(accessor, ui, wndKey)
        {
        }
        DAVA::TArc::ControlDescriptorBuilder<Fields> fields;
    };
    ScaleComboBox(const Params& params, DAVA::TArc::ContextAccessor* accessor, DAVA::Reflection model, QWidget* parent = nullptr);

private:
    void CurrentIndexChanged(int newCurrentItem);
    void EditingFinished();

    QString ValueToString(DAVA::float32 value) const;
    DAVA::float32 StringToValue(const QString& text) const;

    void SetupControl();
    void UpdateControl(const DAVA::TArc::ControlDescriptor& changedfields) override;

    void CreateItems(const DAVA::Reflection& fieldEnumerator);
    void SetCurrentValue(const DAVA::Any& value);

    bool updateControlProceed = false;
    DAVA::TArc::QtConnections connections;
};
