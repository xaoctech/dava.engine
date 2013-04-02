#ifndef STATEPROPERTYGRIDWIDGET_H
#define STATEPROPERTYGRIDWIDGET_H

#include <QWidget>

#include "basepropertygridwidget.h"
#include "StateComboBoxItemDelegate.h"

namespace Ui {
class StatePropertyGridWidget;
}

class QListWidgetItem;
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
	void SelectMultiplyStates(Vector<UIControl::eControlState> selectedStates);
	void SizeChanged(bool expanded);

protected slots:
	void OnExpandButtonClicked();
	void OnSelectAllStateChanged(int state);
	void OnListItemChanged(QListWidgetItem* item);
    void OnCurrrentIndexChanged(int index);

protected:
    // Fill the combobox with the states.
    void FillStatesList();

    // These methods are called when property change is succeeded/failed.
    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);

    // Markup the "dirty" states in the States combo.
    void MarkupDirtyStates();

	void SetExpandState(bool expanded);

	bool eventFilter(QObject *target, QEvent *event);

private:
	enum eCheckedState
	{
		STATE_EVERY_ITEM_UNCHECKED = 0,
		STATE_EVERY_ITEM_CHECKED,
		STATE_PARTIALLY_CHECKED,
		STATE_COUNT
	};

	bool expanded;

	void UpdateState();
	void SetAllChecked(Qt::CheckState state);
	void MultiplyStateSelectionChanged();
	eCheckedState GetCheckedState();

    Ui::StatePropertyGridWidget *ui;
    StateComboBoxItemDelegate stateComboboxItemDelegate;
};

#endif // STATEPROPERTYGRIDWIDGET_H
