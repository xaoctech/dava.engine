#ifndef SPINNERPROPERTYGRIDWIDGET_H
#define SPINNERPROPERTYGRIDWIDGET_H

#include "basepropertygridwidget.h"

namespace Ui {
class SpinnerPropertyGridWidget;
}

class SpinnerPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit SpinnerPropertyGridWidget(QWidget *parent = 0);
    ~SpinnerPropertyGridWidget();

	virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::SpinnerPropertyGridWidget *ui;
};

#endif // SPINNERPROPERTYGRIDWIDGET_H
