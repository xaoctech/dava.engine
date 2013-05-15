#ifndef SLIDERPROPERTYGRIDWIDGET_H
#define SLIDERPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class SliderPropertyGridWidget;
}

class SliderPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit SliderPropertyGridWidget(QWidget *parent = 0);
    ~SliderPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::SliderPropertyGridWidget *ui;
	
	QString GetSpritePathForButton(QWidget *senderWidget);
	
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
    // Use this slot to update value on Value Spin according to slider position
    void OnSliderValueChanged(int);
};

#endif // SLIDERPROPERTYGRIDWIDGET_H
