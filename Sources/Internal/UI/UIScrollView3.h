//
//  UIScrollView.h
//  Framework
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#ifndef __DAVAENGINE_UI_SCROLLVIEW_H__
#define __DAVAENGINE_UI_SCROLLVIEW_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIScrollBar.h"
#include "UI/ScrollHelper.h"

namespace DAVA 
{

class UIScrollView : public UIControl , public UIScrollBarDelegate
{
public:

	UIScrollView();
	UIScrollView(const Rect &rect, bool rectInAbsoluteCoordinates = false);
	
	
	virtual void AddControl(UIControl *control);
	
	// Gettes/Setters for scroll positions
	// Horizontal
	void SetHorizontalScrollPosition(float32 position);
	float32 GetHorizontalScrollPosition();
	// Vertical
	void SetVerticalScrollPosition(float32 position);
	float32 GetVerticalScrollPosition();

    void ScrollToElement(int32 index);
    float32 GetScrollPosition();
    void SetScrollPosition(float32 newScrollPos);
	void ResetScrollPosition();
	void Refresh();
	
	void SetSlowDownTime(float newValue);//sets how fast reduce speed (for example 0.25 reduces speed to zero for the 0.25 second ). To remove inertion effect set tihs value to 0
	void SetBorderMoveModifer(float newValue);//sets how scrolling element moves after reachig a border (0.5 as a default). To remove movement effect after borders set thus value to 0

	void SetTouchHoldDelta(int32 holdDelta);//the amount of pixels user must move the finger on the button to switch from button to scrolling (default 30)
	int32 GetTouchHoldDelta();

	void ScrollTo(float delta);

	virtual List<UIControl* >& GetRealChildren();

	virtual void SystemWillAppear(); // Internal method used by ControlSystem
	
	virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
	
	// Scrollbar delegate implementation
    virtual float32 VisibleAreaSize(UIScrollBar *forScrollBar);
    virtual float32 TotalAreaSize(UIScrollBar *forScrollBar);
    virtual float32 ViewPosition(UIScrollBar *forScrollBar);
    virtual void OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition);

	virtual UIControl *Clone();

protected:
	void InitAfterYaml();
	virtual ~UIScrollView();

	void FullRefresh();

	virtual void Update(float32 timeElapsed);
	
	virtual void Input(UIEvent *currentInput);
	virtual bool SystemInput(UIEvent *currentInput);// Internal method used by ControlSystem

	virtual void SetRect(const Rect &rect, bool rectInAbsoluteCoordinates = FALSE);

	virtual void Draw(const UIGeometricData &geometricData);

	void OnSelectEvent(BaseObject *pCaller, void *pUserData, void *callerData);

	UIControl *scrollContainer;
	
	int mainTouch;
	
	ScrollHelper *scroll;
	
	UIScrollBar *scrollBar;
	
	float oldPos;
	float newPos;
	bool lockTouch;
	
	int32 touchHoldSize;
	
	bool needRefresh;

	float32 verticalScrollPosition;
	float32 horizontalScrollPosition;
};

};

#endif /* defined(__DAVAENGINE_UI_SCROLLVIEW_H__) */
