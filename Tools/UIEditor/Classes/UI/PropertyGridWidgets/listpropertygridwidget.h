#ifndef LISTPROPERTYGRIDWIDGET_H
#define LISTPROPERTYGRIDWIDGET_H

#include <QWidget>
#include <QList>
#include "basepropertygridwidget.h"

namespace Ui {
class ListPropertyGridWidget;
}

class ListPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit ListPropertyGridWidget(QWidget *parent = 0);
    ~ListPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();
private:
    Ui::ListPropertyGridWidget *ui;
	
	// Fill the combo with appropriate values.
    void FillCombobox();
	QList<int> nodeIDList;
	
protected:
    // Background Control contains Comboboxes which should be processed in the specific way.
    virtual void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value);
    virtual void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);

    // Handler for the custom combobox values.
    void CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value);
};

#endif // LISTPROPERTYGRIDWIDGET_H
