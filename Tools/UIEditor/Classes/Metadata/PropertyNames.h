//
//  PropertyNames.h
//  UIEditor
//
//  Created by Yuri Coder on 11/7/12.
//
//

#ifndef __UIEditor__PropertyNames__
#define __UIEditor__PropertyNames__

#include <QString>
namespace DAVA {
namespace PropertyNames
{
    // Property Names for different Properties.
    static const char* LOCALIZED_TEXT_KEY_PROPERTY_NAME = "LocalizedTextKey";
    static const char* FONT_PROPERTY_NAME = "Font";
    static const char* FONT_SIZE_PROPERTY_NAME = "FontSize";
    static const char* FONT_COLOR_PROPERTY_NAME = "FontColor";
    
    static const char* BACKGROUND_COLOR_PROPERTY_NAME = "BackgroundColor";
    static const char* SPRITE_PROPERTY_NAME = "Sprite";
    static const char* SPRITE_FRAME_PROPERTY_NAME = "SpriteFrame";
    
    static const char* DRAW_TYPE_PROPERTY_NAME = "DrawType";
    static const char* COLOR_INHERIT_TYPE_PROPERTY_NAME = "ColorInheritType";
    static const char* ALIGN_PROPERTY_NAME = "Align";
    
    static const char* TEXT_PROPERTY_NAME = "Text";
    static const char* TEXT_COLOR_PROPERTY_NAME = "TextColor";
	
	//Slider properties
	static const char* SLIDER_VALUE_PROPERTY_NAME = "SliderValue";
	static const char* SLIDER_MIN_VALUE_PROPERTY_NAME = "SliderMinValue";
	static const char* SLIDER_MAX_VALUE_PROPERTY_NAME = "SliderMaxValue";
	static const char* SLIDER_THUMB_SPRITE_PROPERTY_NAME = "SliderThumbSprite";
	static const char* SLIDER_THUMB_SPRITE_FRAME_PROPERTY_NAME = "SliderThumbSpriteFrame";
	static const char* SLIDER_MIN_SPRITE_PROPERTY_NAME = "SliderMinSprite";
	static const char* SLIDER_MIN_SPRITE_FRAME_PROPERTY_NAME = "SliderMinSpriteFrame";
	static const char* SLIDER_MIN_DRAW_TYPE_PROPERTY_NAME = "SliderMinDrawType";
	static const char* SLIDER_MAX_SPRITE_PROPERTY_NAME = "SliderMaxSprite";
	static const char* SLIDER_MAX_SPRITE_FRAME_PROPERTY_NAME = "SliderMaxSpriteFrame";
	static const char* SLIDER_MAX_DRAW_TYPE_PROPERTY_NAME = "SliderMaxDrawType";
}
};

#endif /* defined(__UIEditor__PropertyNames__) */
