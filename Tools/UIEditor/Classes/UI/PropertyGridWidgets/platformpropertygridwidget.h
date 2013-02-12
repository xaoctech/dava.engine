#ifndef PLATFORMPROPERTYGRIDWIDGET_H
#define PLATFORMPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class PlatformPropertyGridWidget;
}

class PlatformPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit PlatformPropertyGridWidget(QWidget *parent = 0);
    ~PlatformPropertyGridWidget();
    
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::PlatformPropertyGridWidget *ui;
};

#endif // PLATFORMPROPERTYGRIDWIDGET_H
