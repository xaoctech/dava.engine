#ifndef SCROLLVIEWPROPERTYGRIDWIDGET_H
#define SCROLLVIEWPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class ScrollViewPropertyGridWidget;
}

class ScrollViewPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit ScrollViewPropertyGridWidget(QWidget *parent = 0);
    ~ScrollViewPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::ScrollViewPropertyGridWidget *ui;

protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();

    // Change value of double spin box
    virtual void ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget, const PROPERTYGRIDWIDGETSITER &iter,
                                                               const double value);
    // Update of internal propeperties
    virtual void UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
                                                                const QMetaProperty& curProperty);
private slots:
    // Use this slot to update value on Horizontal or Vertical Scroll position spin according to appropritate slider's position
    void OnSliderValueChanged(int);
};

#endif // SCROLLVIEWPROPERTYGRIDWIDGET_H
