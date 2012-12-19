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
    explicit ControlPropertyGridWidget(QWidget *parent = 0);
    ~ControlPropertyGridWidget();

    // Initialize with control and metadata.
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();
    
private:
    Ui::ControlPropertyGridWidget *ui;
};

#endif // CONTROLPROPERTYGRIDWIDGET_H
