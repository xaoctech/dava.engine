#ifndef RECTPROPERTYGRIDWIDGET_H
#define RECTPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class RectPropertyGridWidget;
}

class RectPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit RectPropertyGridWidget(QWidget *parent = 0);
    ~RectPropertyGridWidget();
    
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
    virtual void OnPropertiesChangedFromExternalSource();
	virtual void HandleChangePropertySucceeded(const QString& propertyName);

private:
    Ui::RectPropertyGridWidget *ui;
	void UpdateWidgetStates(bool updateHorizontalWidgets = true);
	void UpdateHorizontalWidgetsState();
	void UpdateVerticalWidgetsState();
	bool IsTwoAlignsEnabled(bool first, bool center, bool second);
};

#endif // RECTPROPERTYGRIDWIDGET_H
