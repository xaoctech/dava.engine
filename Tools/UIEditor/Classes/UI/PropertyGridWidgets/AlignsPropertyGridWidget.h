#ifndef ALIGNSPROPERTYGRIDWIDGET_H
#define ALIGNSPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class AlignsPropertyGridWidget;
}

class AlignsPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit AlignsPropertyGridWidget(QWidget *parent = 0);
    ~AlignsPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

public:
    Ui::AlignsPropertyGridWidget *ui;	
    

protected:
	virtual void OnPropertiesChangedFromExternalSource();
	virtual void UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget, const QMetaProperty& curProperty);
	virtual void HandleChangePropertySucceeded(const QString& propertyName);

	void UpdateCheckBoxSates();
	void UpdateSpinBoxState(QCheckBox *buddyWidget);
	
    typedef Map<QCheckBox*, QSpinBox*> ALIGNWIDGETSMAP;
    typedef ALIGNWIDGETSMAP::iterator ALIGNWIDGETSMAPITER;
    typedef ALIGNWIDGETSMAP::const_iterator ALIGNWIDGETSMAPCONSTITER;
	
	ALIGNWIDGETSMAP alignWidgetsMap;	
};

#endif // ALIGNSPROPERTYGRIDWIDGET_H
