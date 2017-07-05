#pragma once

#include "TArc/Controls/ControlProxy.h"

#include <QWidget>

class QLayout;

namespace DAVA
{
namespace TArc
{
class Widget : private QWidget, public ControlProxy
{
public:
    explicit Widget(QWidget* parent = nullptr);

    void SetLayout(QLayout* layout);
    void AddControl(ControlProxy* control, Qt::Alignment alignment = Qt::Alignment());
    void HandleControl(ControlProxy* control);

    void ForceUpdate() override;
    void TearDown() override;
    QWidget* ToWidgetCast() override;

private:
    Vector<ControlProxy*> controls;
};
} // namespace TArc
} // namespace DAVA