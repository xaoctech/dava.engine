//
//  BackgroundGridWidgetHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/31/12.
//
//

#include "BackgroundGridWidgetHelper.h"
using namespace DAVA;

const BackgroundGridWidgetHelper::DrawTypesData BackgroundGridWidgetHelper::drawTypesData[] =
{
    {UIControlBackground::DRAW_ALIGNED,                  "Aligned"},
    {UIControlBackground::DRAW_SCALE_TO_RECT,            "Scale to rect"},
    {UIControlBackground::DRAW_SCALE_PROPORTIONAL,       "Scale Proportional"},
    {UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE,   "Scale Proportional One (\?\?\?)"},
    {UIControlBackground::DRAW_FILL,                     "Fill"},
    {UIControlBackground::DRAW_STRETCH_HORIZONTAL,       "Stretch Horizontal"},
    {UIControlBackground::DRAW_STRETCH_VERTICAL,         "Stretch Vertical"},
    {UIControlBackground::DRAW_STRETCH_BOTH  ,            "Stretch Both"}
};

const BackgroundGridWidgetHelper::ColorInheritTypesData BackgroundGridWidgetHelper::colorInheritTypesData[] =
{
    {UIControlBackground::COLOR_MULTIPLY_ON_PARENT,     "Multiply on Parent"},
    {UIControlBackground::COLOR_ADD_TO_PARENT,          "Add to Parent"},
    {UIControlBackground::COLOR_REPLACE_TO_PARENT,      "Replace to Parent"},
    {UIControlBackground::COLOR_IGNORE_PARENT,          "Ignore Parent"},
    {UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY,    "Multiply Alpha only"},
    {UIControlBackground::COLOR_REPLACE_ALPHA_ONLY,     "Replace Alpha only"}
};

const BackgroundGridWidgetHelper::SpriteModificationTypesData BackgroundGridWidgetHelper::spriteModificationTypesData[] =
{
	{0,						"Original"},
	{ESM_HFLIP,				"Horizontal"},
	{ESM_VFLIP,				"Vertical"},
	{ESM_HFLIP|ESM_VFLIP,	"Both"}

};

const BackgroundGridWidgetHelper::AlignTypesData BackgroundGridWidgetHelper::alignTypesData[] =
{
    {ALIGN_LEFT | ALIGN_TOP,        "Left & Top"},
    {ALIGN_LEFT | ALIGN_VCENTER,    "Left & Vert Center"},
    {ALIGN_LEFT | ALIGN_BOTTOM,     "Left & Bottom"},

	{ALIGN_HCENTER | ALIGN_TOP,     "Horz Center & Top"},
    {ALIGN_HCENTER | ALIGN_VCENTER, "Horz Center & Vert Center"},
    {ALIGN_HCENTER | ALIGN_BOTTOM,  "Horz Center & Bottom"},

    {ALIGN_RIGHT | ALIGN_TOP,       "Right & Top"},
    {ALIGN_RIGHT | ALIGN_VCENTER,   "Right & Vert Center"},
    {ALIGN_RIGHT | ALIGN_BOTTOM,    "Right & Bottom"},
    
    {ALIGN_HJUSTIFY,                "Horizontal Justify"}
};

int BackgroundGridWidgetHelper::GetDrawTypesCount()
{
    return sizeof(drawTypesData)/sizeof(*drawTypesData);
}

UIControlBackground::eDrawType BackgroundGridWidgetHelper::GetDrawType(int index)
{
    if (ValidateDrawTypeIndex(index) == false)
    {
        return  drawTypesData[0].drawType;
    }
    
    return drawTypesData[index].drawType;
}

QString BackgroundGridWidgetHelper::GetDrawTypeDesc(int index)
{
    if (ValidateDrawTypeIndex(index) == false)
    {
        return  drawTypesData[0].drawTypeDesc;
    }
    
    return drawTypesData[index].drawTypeDesc;
}

QString BackgroundGridWidgetHelper::GetDrawTypeDescByType(UIControlBackground::eDrawType drawType)
{
    int count = GetDrawTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (drawTypesData[i].drawType == drawType)
        {
            return drawTypesData[i].drawTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Draw Type %i!", drawType);
    return QString();
}

int BackgroundGridWidgetHelper::GetColorInheritTypesCount()
{
    return sizeof(colorInheritTypesData)/sizeof(*colorInheritTypesData);
}

UIControlBackground::eColorInheritType BackgroundGridWidgetHelper::GetColorInheritType(int index)
{
    if (ValidateColorInheritTypeIndex(index) == false)
    {
        return  colorInheritTypesData[0].colorInheritType;
    }
    
    return colorInheritTypesData[index].colorInheritType;
}

QString BackgroundGridWidgetHelper::GetColorInheritTypeDesc(int index)
{
    if (ValidateColorInheritTypeIndex(index) == false)
    {
        return  colorInheritTypesData[0].colorInheritTypeDesc;
    }
    
    return colorInheritTypesData[index].colorInheritTypeDesc;
}

QString BackgroundGridWidgetHelper::GetColorInheritTypeDescByType(UIControlBackground::eColorInheritType inheritType)
{
    int count = GetColorInheritTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (colorInheritTypesData[i].colorInheritType == inheritType)
        {
            return colorInheritTypesData[i].colorInheritTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Color Inherit Type %i!", inheritType);
    return QString();
}

int BackgroundGridWidgetHelper::GetAlignTypesCount()
{
    return sizeof(alignTypesData)/sizeof(*alignTypesData);
}

int BackgroundGridWidgetHelper::GetAlignType(int index)
{
    if (ValidateAlginTypeIndex(index) == false)
    {
        return alignTypesData[0].alignType;
    }
    
    return alignTypesData[index].alignType;
}

QString BackgroundGridWidgetHelper::GetAlignTypeDesc(int index)
{
    if (ValidateAlginTypeIndex(index) == false)
    {
        return alignTypesData[0].alignTypeDesc;
    }
    
    return alignTypesData[index].alignTypeDesc;
}

// Get the Align Type Description by the Value.
QString BackgroundGridWidgetHelper::GetAlignTypeDescByType(int alignType)
{
    int count = GetAlignTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (alignTypesData[i].alignType == alignType)
        {
            return alignTypesData[i].alignTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Align Type %i!", alignType);
    return QString();
}

int BackgroundGridWidgetHelper::GetModificationType(int index)
{
	if (ValidateSpriteModificationIndex(index) == false)
    {
		return  spriteModificationTypesData[0].spriteModificationType;
    }
    
	return spriteModificationTypesData[index].spriteModificationType;
}

QString BackgroundGridWidgetHelper::GetModificationTypeDesc(int index)
{
	if (ValidateSpriteModificationIndex(index) == false)
    {
		return  spriteModificationTypesData[0].spriteModificationTypeDescription;
    }
    
    return spriteModificationTypesData[index].spriteModificationTypeDescription;
}

int BackgroundGridWidgetHelper::GetModificationTypesCount()
{
    return sizeof(spriteModificationTypesData)/sizeof(*spriteModificationTypesData);
}

QString BackgroundGridWidgetHelper::GetModificationTypeDescByType(int modifType)
{
	int count = GetModificationTypesCount();
    for (int i = 0; i < count; i ++)
    {
		if (spriteModificationTypesData[i].spriteModificationType == modifType)
        {
			return spriteModificationTypesData[i].spriteModificationTypeDescription;
        }
    }
    
    Logger::Error("Unknown/unsupported Modification Type %i!", modifType);
    return QString();
}
bool BackgroundGridWidgetHelper::ValidateDrawTypeIndex(int index)
{
    if (index < 0 || index >= GetDrawTypesCount())
    {
        Logger::Error("Draw Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateColorInheritTypeIndex(int index)
{
    if (index < 0 || index >= GetColorInheritTypesCount())
    {
        Logger::Error("Color Inherit Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateAlginTypeIndex(int index)
{
    if (index < 0 || index >= GetAlignTypesCount())
    {
        Logger::Error("Align Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateSpriteModificationIndex(int index)
{
	if (index < 0 || index >= GetModificationTypesCount())
    {
        Logger::Error("Sprite modification index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

