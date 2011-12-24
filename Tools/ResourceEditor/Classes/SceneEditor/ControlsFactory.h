#ifndef __CONTROLS_FACTORY_H__
#define __CONTROLS_FACTORY_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ControlsFactory 
{

public:
    
    static UIButton *CreateButton(const Rect & rect, const WideString &buttonText);
    static void CustomizeButton(UIButton *btn, const WideString &buttonText);

    static UIButton *CreateCloseWindowButton(const Rect & rect);
    static void CustomizeCloseWindowButton(UIButton *btn);
    
    static Font* CreateFontLight();
    static void CustomizeFontLight(Font *font);
    static Font* CreateFontDark();
    static void CustomizeFontDark(Font *font);

    static void CustomizeScreenBack(UIControl *screen);
    
    static UIControl * CreateLine(const Rect & rect);
    
    static void CusomizeBottomLevelControl(UIControl *c);

    static void CusomizeTopLevelControl(UIControl *c);

    static void CusomizeListControl(UIControl *c);
    
    static UIControl *CreatePanelControl(const Rect & rect);
    static void CustomizePanelControl(UIControl *c);
    
    static void CustomizeExpandButton(UIButton *btn);

    static void CustomizeListCell(UIListCell *c);
    static void CustomizeSceneGraphCell(UIHierarchyCell *c);
    
    static void CustomizeMenuPopupCell(UIListCell *c, const WideString &text);
    
    static void CustomizePropertyCell(UIControl *c, bool isActivePart);
    static void CustomizeEditablePropertyCell(UIControl *c);
    static void CustomizeUneditablePropertyCell(UIControl *c);
};



#endif // __CONTROLS_FACTORY_H__