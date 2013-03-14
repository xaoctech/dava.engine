#ifndef BACKGROUNDPROPERTYGRIDWIDGET_H
#define BACKGROUNDPROPERTYGRIDWIDGET_H

#include <QWidget>

#include "basepropertygridwidget.h"
#include "StateComboBoxItemDelegate.h"

namespace Ui {
class BackGroundPropertyGridWidget;
}

class BackGroundPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit BackGroundPropertyGridWidget(QWidget *parent = 0);
    ~BackGroundPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::BackGroundPropertyGridWidget *ui;

protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();

    // Background Control contains Comboboxes which should be processed in the specific way.
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);

    // Fill the combos with appropriate values.
    void FillComboboxes();

    // Handler for the custom combobox values.
    void CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value);

    // Pre-process the sprite name.
    QString PreprocessSpriteName(const QString& rawSpriteName);

	//handle elements according drawType
	void HandleDrawTypeComboBox();

private slots:
    void OpenSpriteDialog();
    void RemoveSprite();
};

#endif // BACKGROUNDPROPERTYGRIDWIDGET_H
