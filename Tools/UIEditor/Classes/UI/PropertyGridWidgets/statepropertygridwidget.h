#ifndef STATEPROPERTYGRIDWIDGET_H
#define STATEPROPERTYGRIDWIDGET_H

#include <QWidget>

#include "basepropertygridwidget.h"
#include "StateComboBoxItemDelegate.h"

namespace Ui {
class StatePropertyGridWidget;
}

class StatePropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit StatePropertyGridWidget(QWidget *parent = 0);
    ~StatePropertyGridWidget();

    // Initialize with the metatada assigned.
    virtual void Initialize(BaseMetadata* metaData);
    virtual void Cleanup();

signals:
    void SelectedStateChanged(UIControl::eControlState newState);

protected slots:
    void OnCurrrentIndexChanged(int index);

protected:
    // Fill the combobox with the states.
    void FillStatesList();

    // These methods are called when property change is succeeded/failed.
    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);

    // Markup the "dirty" states in the States combo.
    void MarkupDirtyStates();

private:
    Ui::StatePropertyGridWidget *ui;
    StateComboBoxItemDelegate stateComboboxItemDelegate;
};

#endif // STATEPROPERTYGRIDWIDGET_H
