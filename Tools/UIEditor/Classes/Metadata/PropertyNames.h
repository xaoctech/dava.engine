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
class PropertyNames
{
public:
    // Property Names for different Properties.
	static const char* SIZE_X;
	static const char* SIZE_Y;
	
    static const char* LOCALIZED_TEXT_KEY_PROPERTY_NAME;
    static const char* FONT_PROPERTY_NAME;
    static const char* FONT_SIZE_PROPERTY_NAME;
    static const char* FONT_COLOR_PROPERTY_NAME;
    
    static const char* BACKGROUND_COLOR_PROPERTY_NAME;
    static const char* SPRITE_PROPERTY_NAME;
    static const char* SPRITE_FRAME_PROPERTY_NAME;
	static const char* SPRITE_MODIFICATION_PROPERTY_NAME;

	static const char* STRETCH_HORIZONTAL_PROPERTY_NAME;
	static const char* STRETCH_VERTICAL_PROPERTY_NAME;
    
    static const char* DRAW_TYPE_PROPERTY_NAME;
    static const char* COLOR_INHERIT_TYPE_PROPERTY_NAME;
    static const char* ALIGN_PROPERTY_NAME;
    
    static const char* TEXT_PROPERTY_NAME;
    static const char* TEXT_COLOR_PROPERTY_NAME;
	static const char* TEXT_ALIGN_PROPERTY_NAME;

	static const char* SHADOW_OFFSET_X;
	static const char* SHADOW_OFFSET_Y;
	static const char* SHADOW_COLOR;

	// Slider properties
	static const char* SLIDER_VALUE_PROPERTY_NAME;
	static const char* SLIDER_MIN_VALUE_PROPERTY_NAME;
	static const char* SLIDER_MAX_VALUE_PROPERTY_NAME;
	static const char* SLIDER_THUMB_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_THUMB_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MIN_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_MIN_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MIN_DRAW_TYPE_PROPERTY_NAME;
	static const char* SLIDER_MAX_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_MAX_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MAX_DRAW_TYPE_PROPERTY_NAME;
	
	// Align properties
	static const char* LEFT_ALIGN;
	static const char* LEFT_ALIGN_ENABLED;
	static const char* HCENTER_ALIGN;
	static const char* HCENTER_ALIGN_ENABLED;
	static const char* RIGHT_ALIGN;
	static const char* RIGHT_ALIGN_ENABLED;
	static const char* TOP_ALIGN;
	static const char* TOP_ALIGN_ENABLED;
	static const char* VCENTER_ALIGN;
	static const char* VCENTER_ALIGN_ENABLED;
	static const char* BOTTOM_ALIGN;
	static const char* BOTTOM_ALIGN_ENABLED;
	
	// UI Scroll View properties
	static const char* HORIZONTAL_SCROLL_POSITION;
	static const char* VERTICAL_SCROLL_POSITION;
	static const char* SCROLL_CONTENT_SIZE_X;
	static const char* SCROLL_CONTENT_SIZE_Y;

	// UI Spinner properties.
	static const char* UISPINNER_PREV_BUTTON_TEXT;
	static const char* UISPINNER_NEXT_BUTTON_TEXT;

	// Custom Control properties.
	static const char* CUSTOM_CONTROL_NAME;
};

};

#endif /* defined(__UIEditor__PropertyNames__) */
