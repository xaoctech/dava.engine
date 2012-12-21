#ifndef SCREENPROPERTYGRIDWIDGET_H
#define SCREENPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class ScreenPropertyGridWidget;
}

class ScreenPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit ScreenPropertyGridWidget(QWidget *parent = 0);
    ~ScreenPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::ScreenPropertyGridWidget *ui;
};

#endif // SCREENPROPERTYGRIDWIDGET_H
