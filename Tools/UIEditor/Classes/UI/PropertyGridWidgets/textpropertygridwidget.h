#ifndef TEXTPROPERTYGRIDWIDGET_H
#define TEXTPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
class TextPropertyGridWidget;
}

class TextPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit TextPropertyGridWidget(QWidget *parent = 0);
    ~TextPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
    // Update the widget with Localization Value when the key is changed.
    void UpdateLocalizationValue();

    // Property change succeeded/failed.
    virtual void ProcessPushButtonClicked(QPushButton* senderWidget);
    virtual void HandleLineEditEditingFinished(QLineEdit* senderWidget);
    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    virtual void HandleChangePropertyFailed(const QString& propertyName);
    
    //Update of internal propeperties
    virtual void UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty& curProperty);

    // Handle UI Control State is changed - needed for updating Localization Text.
    virtual void HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates);

private:
    Ui::TextPropertyGridWidget *ui;
};

#endif // TEXTPROPERTYGRIDWIDGET_H
