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

private:
    Ui::RectPropertyGridWidget *ui;
};

#endif // RECTPROPERTYGRIDWIDGET_H
