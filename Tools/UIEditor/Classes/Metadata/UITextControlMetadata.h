#ifndef __UIEditor__TextControlNodeMetadata__
#define __UIEditor__TextControlNodeMetadata__

#include "UIControlMetadata.h"
#include "UI/UIStaticText.h"

#include <QColor>

namespace DAVA {

// Metadata class for DAVA text-aware controls, responsible for the localization.
class UITextControlMetadata : public UIControlMetadata
{
    Q_OBJECT

    // Font Properties
    Q_PROPERTY(Font* Font READ GetFont WRITE SetFont);
    Q_PROPERTY(float FontSize READ GetFontSize WRITE SetFontSize);
    Q_PROPERTY(QColor FontColor READ GetFontColor WRITE SetFontColor);

    Q_PROPERTY(QString LocalizedTextKey READ GetLocalizedTextKey WRITE SetLocalizedTextKey);
	
	Q_PROPERTY(float ShadowOffsetX READ GetShadowOffsetX WRITE SetShadowOffsetX);
	Q_PROPERTY(float ShadowOffsetY READ GetShadowOffsetY WRITE SetShadowOffsetY);
	Q_PROPERTY(QColor ShadowColor READ GetShadowColor WRITE SetShadowColor);
	Q_PROPERTY(int TextAlign READ GetTextAlign WRITE SetTextAlign);

public:
    UITextControlMetadata(QObject* parent = 0);
    QString GetLocalizedTextValue();
    
protected:
    virtual QString GetLocalizedTextKey() const;
    virtual void SetLocalizedTextKey(const QString& value);
    
    // Getters/setters.
    virtual Font * GetFont() = 0;
    virtual void SetFont(Font * font) = 0;

	virtual int GetTextAlign() = 0;
    virtual void SetTextAlign(int align) = 0;
    
    virtual float GetFontSize() const = 0;
    virtual void SetFontSize(float fontSize) = 0;

    virtual QColor GetFontColor() const = 0;
    virtual void SetFontColor(const QColor& value) = 0;
	
	virtual float GetShadowOffsetX() const = 0;
	virtual void SetShadowOffsetX(float offset) = 0;
	
	virtual float GetShadowOffsetY() const = 0;
	virtual void SetShadowOffsetY(float offset) = 0;
	
	virtual QColor GetShadowColor() const = 0;
	virtual void SetShadowColor(const QColor& value) = 0;
	
	Vector2 GetOffsetX(const Vector2& currentOffset, float offsetX);
	Vector2 GetOffsetY(const Vector2& currentOffset, float offsetY);

    // Get the localized text for particular control state.
    QString GetLocalizedTextKeyForState(UIControl::eControlState controlState) const;
    
    // Update the static text extra data based on update style.
    void UpdateStaticTextExtraData(UIStaticText* staticText, UIControl::eControlState state,
                                   HierarchyTreeNodeExtraData& extraData,
                                   eExtraDataUpdateStyle updateStyle);
};

};

#endif /* defined(__UIEditor__TextControlNodeMetadata__)*/
