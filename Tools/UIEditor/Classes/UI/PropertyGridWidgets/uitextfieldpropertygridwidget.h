#ifndef UITEXTFIELDPROPERTYGRIDWIDGET_H
#define UITEXTFIELDPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class UITextFieldPropertyGridWidget;
}

class UITextFieldPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit UITextFieldPropertyGridWidget(QWidget *parent = 0);
    ~UITextFieldPropertyGridWidget();

    virtual void ProcessPushButtonClicked(QPushButton* senderWidget);
    //Update of internal propeperties
    virtual void UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty& curProperty);
    
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

private:
    Ui::UITextFieldPropertyGridWidget *ui;
};

#endif // UITEXTFIELDPROPERTYGRIDWIDGET_H
