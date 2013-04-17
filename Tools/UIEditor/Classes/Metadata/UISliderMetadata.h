//
//  UISliderMetadata.h
//  UIEditor
//
//  Created by Denis Bespalov on 12/24/12.
//
//

#ifndef UIEditor_UISliderMetadata_h
#define UIEditor_UISliderMetadata_h

#include "UIControlMetadata.h"
#include "UI/UISlider.h"

namespace DAVA {

// Metadata class for DAVA text-aware controls, responsible for the localization.
class UISliderMetadata : public UIControlMetadata
{
	Q_OBJECT

    // Slider Properties
    Q_PROPERTY(float SliderValue READ GetSliderValue WRITE SetSliderValue);
	Q_PROPERTY(float SliderMinValue READ GetSliderMinValue WRITE SetSliderMinValue);
	Q_PROPERTY(float SliderMaxValue READ GetSliderMaxValue WRITE SetSliderMaxValue);
	
	// Thumb sprite and frame
	Q_PROPERTY(QString SliderThumbSprite READ GetSliderThumbSprite WRITE SetSliderThumbSprite);
    Q_PROPERTY(int SliderThumbSpriteFrame READ GetSliderThumbSpriteFrame WRITE SetSliderThumbSpriteFrame);
	
	// Min sprite, frame and drawtype
	Q_PROPERTY(QString SliderMinSprite READ GetSliderMinSprite WRITE SetSliderMinSprite);
	Q_PROPERTY(int SliderMinSpriteFrame READ GetSliderMinSpriteFrame WRITE SetSliderMinSpriteFrame);
	Q_PROPERTY(int SliderMinDrawType READ GetSliderMinDrawType WRITE SetSliderMinDrawType);
	
	// Max sprite, frame and drawtype and leftrigth stratch cap
	Q_PROPERTY(QString SliderMaxSprite READ GetSliderMaxSprite WRITE SetSliderMaxSprite);
	Q_PROPERTY(int SliderMaxSpriteFrame READ GetSliderMaxSpriteFrame WRITE SetSliderMaxSpriteFrame);
	Q_PROPERTY(int SliderMaxDrawType READ GetSliderMaxDrawType WRITE SetSliderMaxDrawType);
	  
public:
    UISliderMetadata(QObject* parent = 0);
    
protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);   
    virtual QString GetUIControlClassName() { return "UISlider"; };
	// Override Resize function
	virtual void ApplyResize(const Rect& originalRect, const Rect& newRect);
		
    // Helper to access active UI Slider.
    UISlider* GetActiveUISlider() const;
	
    // Getters/setters.
    float GetSliderValue() const;
	void SetSliderValue(float value);
	
	// Max/Min value
    float GetSliderMinValue() const;
    void SetSliderMinValue(float value);
    float GetSliderMaxValue() const;
    void SetSliderMaxValue(float value);
	
	// Slider thumb sprite and frame
	QString GetSliderThumbSprite() const;
	void SetSliderThumbSprite(QString value);
	int GetSliderThumbSpriteFrame() const;
	void SetSliderThumbSpriteFrame(int value);
	
	// Slider Min sprite, frame and drawtype
	QString GetSliderMinSprite() const;
	void SetSliderMinSprite(QString value);
	int GetSliderMinSpriteFrame() const;
	void SetSliderMinSpriteFrame(int value);
	int GetSliderMinDrawType() const;
	void SetSliderMinDrawType(int value);
	
	// Slider Max sprite, frame and drawtype
	QString GetSliderMaxSprite() const;
	void SetSliderMaxSprite(QString value);
	int GetSliderMaxSpriteFrame() const;
	void SetSliderMaxSpriteFrame(int value);
	int GetSliderMaxDrawType() const;
	void SetSliderMaxDrawType(int value);
};

};

#endif
