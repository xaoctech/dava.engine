#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Qt/QtIcon.h"

#include <Functional/Signal.h>
#include <Reflection/Reflection.h>

#include <QWidget>
#include <QPointer>

class QPaintEvent;
namespace DAVA
{
class FilterWidget : public DAVA::TArc::ControlProxyImpl<QWidget>
{
    using TBase = DAVA::TArc::ControlProxyImpl<QWidget>;

public:
    enum Fields
    {
        Enabled,
        Inversed,
        Title,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    FilterWidget(const Params& params, TArc::DataWrappersProcessor& processor, Reflection model, QWidget* parent = nullptr);

    Signal<> requestRemoving;
    Signal<> updateRequire;

    void ResetModel(Reflection model);

protected:
    void paintEvent(QPaintEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;

    void SetupControl();
    void UpdateControl(const TArc::ControlDescriptor& descriptor) override;

    QIcon GetEnableButtonIcon() const;
    QIcon GetInverseButtonIcon() const;
    QString GetTitle() const;
    void ToggleEnabling();
    void ToggleInversing();
    void RemoveFilter();

private:
    DAVA::TArc::QtConnections connections;
    QPointer<QPushButton> titleButton = nullptr;

    DAVA_REFLECTION(FilterWidget);
};

} // namespace DAVA
