#ifndef TEXTPROPERTYGRIDWIDGET_H
#define TEXTPROPERTYGRIDWIDGET_H

#include <QWidget>
#include "basepropertygridwidget.h"
#include "uitextfieldpropertygridwidget.h"

class QLabel;

class TextPropertyGridWidget : public UITextFieldPropertyGridWidget
{
    Q_OBJECT

public:
    explicit TextPropertyGridWidget(QWidget *parent = 0);
    ~TextPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
    
	virtual void InsertLocalizationFields();

    // Update the widget with Localization Value when the key is changed.
    virtual void UpdateLocalizationValue();

    virtual void HandleChangePropertySucceeded(const QString& propertyName);
    
    virtual void HandleChangePropertyFailed(const QString& propertyName);
    
	// Handle UI Control State is changed - needed for updating Localization Text.
    virtual void HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates);

private:

	QLineEdit *localizationKeyNameLineEdit;
    QLineEdit *localizationKeyTextLineEdit;
	QLabel	*localizationKeyNameLabel;
	QLabel	*localizationKeyTextLabel;
};

#endif // TEXTPROPERTYGRIDWIDGET_H
