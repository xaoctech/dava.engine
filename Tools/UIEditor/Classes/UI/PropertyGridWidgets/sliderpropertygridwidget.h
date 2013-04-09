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
    // Comboboxes hould be processed in the specific way.
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);

    // Fill the combos with appropriate values.
    void FillComboboxes();

    // Handler for the custom combobox values.
    void CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value);
	
	// Update sprite of selected control
	void SetSprite(QWidget *senderWidget, const QString& spritePath);

private slots:
    // Use this slot to update value on Value Spin according to slider position
    void OnSliderValueChanged(int);
    // Add/remove sprites
    void OnOpenSpriteDialog();
    void OnRemoveSprite();
};

#endif // SLIDERPROPERTYGRIDWIDGET_H
