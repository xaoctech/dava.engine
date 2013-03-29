#ifndef CONTROLPROPERTYGRIDWIDGET_H
#define CONTROLPROPERTYGRIDWIDGET_H

#include "BaseMetadata.h"

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class ControlPropertyGridWidget;
}

class ControlPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
	// Control widget states - whether we are displaying info for Default or Custom controls.
	enum eWidgetState
	{
		STATE_DEFAULT_CONTROL,
		STATE_CUSTOM_CONTROL
	};

    explicit ControlPropertyGridWidget(QWidget *parent = 0);
    ~ControlPropertyGridWidget();

    // Initialize with control and metadata.
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
	void ConnectToSignals();
	void DisconnectFromSignals();

	// Change the state of the widget.
	void SetWidgetState(eWidgetState newState, bool forceUpdate = false);

	// Update the properties for Subcontrols.
	void UpdatePropertiesForSubcontrol();

	// Override this method to setup the correct state of the control.
	virtual void UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
													   const QMetaProperty& curProperty);
protected slots:
	void OnMorphToCustomControlClicked();
	void OnResetMorphToCustomControlClicked();

private:
    Ui::ControlPropertyGridWidget *ui;
	
	// Current widget state.
	eWidgetState currentWidgetState;
};

#endif // CONTROLPROPERTYGRIDWIDGET_H
