#ifndef __CONTROLS_FACTORY_H__
#define __CONTROLS_FACTORY_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ControlsFactory 
{
public:
    
    enum eControlSize
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 80,
        
        LEFT_SIDE_WIDTH = 200,
        RIGHT_SIDE_WIDTH = 200,
    };
    
    
public:
    
    static void AddBorder(UIControl *c);
    
    static UIButton *CreateButton(Vector2 pos, const WideString &buttonText);
    static UIButton *CreateButton(const Rect & rect, const WideString &buttonText);
    static void CustomizeButton(UIButton *btn, const WideString &buttonText);

    static UIButton *CreateImageButton(const Rect & rect, const String &imagePath);
    static void CustomizeImageButton(UIButton *btn, const String &imagePath);
    
    static UIButton *CreateCloseWindowButton(const Rect & rect);
    static void CustomizeCloseWindowButton(UIButton *btn);
    
    static Font* GetFontLight();
    static Font* GetFontDark();
    static Font* GetFontError();
    static void CustomizeFontLight(Font *font);
    static void CustomizeFontDark(Font *font);
    static void CustomizeFontError(Font *font);

    static void CustomizeScreenBack(UIControl *screen);
    
    static UIControl * CreateLine(const Rect & rect);
    static UIControl * CreateLine(const Rect & rect, Color color);
    
    static void CusomizeBottomLevelControl(UIControl *c);

    static void CusomizeTopLevelControl(UIControl *c);

    static void CusomizeListControl(UIControl *c);
    
    static UIControl *CreatePanelControl(const Rect & rect, bool addBorder = true);
    static void CustomizePanelControl(UIControl *c, bool addBorder = true);
    
    static void CustomizeExpandButton(UIButton *btn);

    static void CustomizeListCell(UIListCell *c, const WideString &text);
    static void CustomizeListCellAlternative(UIListCell *c, const WideString &text);
    static void CustomizeSceneGraphCell(UIHierarchyCell *c);
    
    static void CustomizeMenuPopupCell(UIListCell *c, const WideString &text);
    
    static void CustomizePropertyCell(UIControl *c, bool isActivePart);
    static void CustomizeEditablePropertyCell(UIControl *c);
    static void CustomizeUneditablePropertyCell(UIControl *c);
    static void CustomizePropertySectionCell(UIControl *c);
    static void CustomizePropertyButtonCell(UIListCell *c);
    
    static void CustomizeDialogFreeSpace(UIControl *c);
    static void CustomizeDialog(UIControl *c);
    
    static void SetScrollbar(UIList *l);
    static void SetScrollbar(UIHierarchy *l);
    
    static Font* fontLight;
    static Font* fontDark;
    static Font* fontError;
};



#endif // __CONTROLS_FACTORY_H__