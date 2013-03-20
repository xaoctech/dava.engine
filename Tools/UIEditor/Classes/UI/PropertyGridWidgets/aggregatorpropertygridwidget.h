#ifndef AGGREGATORPROPERTYGRIDWIDGET_H
#define AGGREGATORPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class AggregatorPropertyGridWidget;
}

class AggregatorPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit AggregatorPropertyGridWidget(QWidget *parent = 0);
    ~AggregatorPropertyGridWidget();
    
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::AggregatorPropertyGridWidget *ui;
};

#endif // AGGREGATORPROPERTYGRIDWIDGET_H
