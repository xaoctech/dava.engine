#ifndef FLAGSPROPERTYGRIDWIDGET_H
#define FLAGSPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class FlagsPropertyGridWidget;
}

class FlagsPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit FlagsPropertyGridWidget(QWidget *parent = 0);
    ~FlagsPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();
private:
    Ui::FlagsPropertyGridWidget *ui;
};

#endif // FLAGSPROPERTYGRIDWIDGET_H
