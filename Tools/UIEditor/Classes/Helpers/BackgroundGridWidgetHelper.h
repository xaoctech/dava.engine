//
//  BackgroundGridWidgetHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 10/31/12.
//
//

#ifndef __UIEditor__BackgroundGridWidgetHelper__
#define __UIEditor__BackgroundGridWidgetHelper__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include <QString>

namespace DAVA {
    
class BackgroundGridWidgetHelper
{
public:
    // Draw Type.
    static int GetDrawTypesCount();
    static UIControlBackground::eDrawType GetDrawType(int index);
    static QString GetDrawTypeDesc(int index);

    // Get the Draw Type description by the Value.
    static QString GetDrawTypeDescByType(UIControlBackground::eDrawType drawType);

    // Color Inherit.
    static int GetColorInheritTypesCount();
    static UIControlBackground::eColorInheritType GetColorInheritType(int index);
    static QString GetColorInheritTypeDesc(int index);

    // Get the Color Inherit Type Description by the Value.
    static QString GetColorInheritTypeDescByType(UIControlBackground::eColorInheritType inheritType);

    //Align.
    static int GetAlignTypesCount();
    static int GetAlignType(int index);
    static QString GetAlignTypeDesc(int index);

    // Get the Align Type Description by the Value.
    static QString GetAlignTypeDescByType(int alignType);

	//SpriteModificationType
	static int GetModificationType(int index);
    static QString GetModificationTypeDesc(int index);
	static int GetModificationTypesCount();

	static QString GetModificationTypeDescByType(int modificationType);

protected:
    // Validate the indexes.
    static bool ValidateDrawTypeIndex(int index);
    static bool ValidateColorInheritTypeIndex(int index);
    static bool ValidateAlginTypeIndex(int index);
	static bool ValidateSpriteModificationIndex(int index);
    
    // Maps.
    struct DrawTypesData
    {
        UIControlBackground::eDrawType drawType;
        const char* drawTypeDesc;
    };
    
    struct ColorInheritTypesData
    {
        UIControlBackground::eColorInheritType colorInheritType;
        const char* colorInheritTypeDesc;
    };

    struct AlignTypesData
    {
        int alignType; // int, not eAlign, since align types combinations are possible.
        const char* alignTypeDesc;
    };

	struct SpriteModificationTypesData
    {
        int spriteModificationType;
        const char* spriteModificationTypeDescription;
    };

    static const DrawTypesData drawTypesData[];
    static const ColorInheritTypesData colorInheritTypesData[];
    static const AlignTypesData alignTypesData[];
	static const SpriteModificationTypesData spriteModificationTypesData[];
};

};

#endif /* defined(__UIEditor__BackgroundGridWidgetHelper__) */
