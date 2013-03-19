#ifndef UITEXTFIELDMETADATA_H
#define UITEXTFIELDMETADATA_H

#include "UITextControlMetadata.h"
#include "UI/UITextField.h"

namespace DAVA {

// Metadata class for DAVA text-field controls.
class UITextFieldMetadata : public UITextControlMetadata
{
    Q_OBJECT

    // Text field text Properties
    Q_PROPERTY(QString Text READ GetText WRITE SetText);
    Q_PROPERTY(QColor TextColor READ GetTextColor WRITE SetTextColor);

public:
    UITextFieldMetadata(QObject* parent = 0);

protected:
	virtual QString GetUIControlClassName() { return "UITextField"; };
	
    // Initialization.
    void InitializeControl(const String& controlName, const Vector2& position);

    // Helper to access active UI Text Field.
    UITextField* GetActiveUITextField() const;
     // Getters/setters.
    QString GetText() const;
    void SetText(const QString& text);
    
    virtual Font * GetFont();
    virtual void SetFont(Font* font);

    virtual float GetFontSize() const;
    virtual void SetFontSize(float fontSize);

    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);

    QColor GetTextColor() const;
    void SetTextColor(const QColor& color);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);
};

};


#endif // UITEXTFIELDMETADATA_H
