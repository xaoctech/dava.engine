#ifndef __CONTROLS_FACTORY_H__
#define __CONTROLS_FACTORY_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ControlsFactory 
{

public:
    
    static UIButton *CreateButton(Rect r, const WideString &buttonText);
    static void CustomizeButton(UIButton *btn, const WideString &buttonText);

    static UIButton *CreateCloseWindowButton(Rect r);
    static void CustomizeCloseWindowButton(UIButton *btn);
    
    static Font* CreateFontLight();
    static void CustomizeFontLight(Font *font);
    static Font* CreateFontDark();
    static void CustomizeFontDark(Font *fontf);

    static void CustomizeScreenBack(UIControl *screen);
    
    static UIControl * CreateLine(Rect r);
    
    static void CusomizeBottomLevelControl(UIControl *c);

    static void CusomizeTopLevelControl(UIControl *c);

    static void CusomizeListControl(UIControl *c);
    
    static UIControl *CreatePanelControl(Rect r);
    static void CustomizePanelControl(UIControl *c);
    
    static void CustomizeExpandButton(UIButton *btn);

    static void CustomizeListCell(UIListCell *c);
    static void CustomizeHierarhyCell(UIHierarchyCell *c);
};



#endif // __CONTROLS_FACTORY_H__