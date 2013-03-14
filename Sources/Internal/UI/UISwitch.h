#ifndef __DAVAENGINE_UI_SWITCH_H__
#define __DAVAENGINE_UI_SWITCH_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"

namespace DAVA
{

class TogglePositionAnimation;

/*
 * buttonLeft.relativePosition.x and buttonRight.relativePosition.x mark movement bounds for 'toggle': it does mean that toggle never leaves 
 * space between pivot points of these controls. So if you want to have toggle that moves between the left edge of 'buttonLeft' and 
 * the right edge of 'buttonRight' set buttonLeft.pivotPoint.x = 0 and buttonRight.pivotPoint.x = buttonRight.size.dx.
 *
 * Default sizes for buttonLeft, buttonRight and toggle are set only once in UISwitch ctor and only if you provide 'rect' argument.
 * After this they never adjust their sizes and it's your responsibility to control this parameter.
 *
 * Don't use GetButtonXXX()->GetSelected() to determine what side is currently selected: GetIsLeftSelected() is what you need.
 */
class UISwitch : public UIControl
{

    friend class TogglePositionAnimation;

public:
    UISwitch(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~UISwitch();

    virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();
    virtual void CopyDataFrom(DAVA::UIControl *srcControl);

    virtual void Input(UIEvent *currentInput);

    bool GetIsLeftSelected() {return isLeftSelected;}
    void SetIsLeftSelected(bool aIsLeftSelected);

    UIButton * GetButtonNext() {return buttonLeft;}
    UIButton * GetButtonPrevious() {return buttonRight;}
    UIButton * GetToggle() {return toggle;}

    /*
     * If tap on any place beside toggle must provoke switch of controls state.
     */
    void SetSwitchOnTapBesideToggle(bool aSwitchOnTapBesideToggle) {switchOnTapBesideToggle = aSwitchOnTapBesideToggle;}
    bool GetSwitchOnTapBesideToggle() {return switchOnTapBesideToggle;}

protected:
    void InternalSetIsLeftSelected(bool aIsLeftSelected, bool changeVisualState);
    void InitControls();
    void ReleaseControls();
    void FindRequiredControls();

    float32 GetToggleUttermostPosition();
    void CheckToggleSideChange();
    void ChangeVisualState();

    UIButton * buttonLeft;
    UIButton * buttonRight;
    UIButton * toggle;

    bool switchOnTapBesideToggle;
    bool isLeftSelected;
};

}
#endif //__DAVAENGINE_UI_SWITCH_H__