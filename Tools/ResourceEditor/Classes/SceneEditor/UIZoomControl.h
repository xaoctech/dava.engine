/*
 *  UIZoomControl.h
 *  MahJongQuest
 *
 *  Created by Dizz on 11/13/09.
 *  Copyright 2009 1. All rights reserved.
 *
 */

#ifndef __UI_ZOOM_CONTROL_H__
#define __UI_ZOOM_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class UIZoomControl : public UIControl
{
public:
	UIZoomControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~UIZoomControl();
	
	void SetScales(float32 minScale, float32 maxScale);
	void SetContentSize(const Vector2 &_contentSize);
    const Vector2 &GetContentSize();
	void SetScale(float currentScale);
    float32 GetScale();
	void SetOffset(const Vector2& offset);
    Vector2 GetOffset();
    
	Animation *	ScrollOffsetAnimation(const Vector2 & _position, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    
    virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    
	virtual void		Update(float32 timeElapsed);
	virtual void		Input(UIEvent * currentTouch);
	virtual void		SystemDraw(const UIGeometricData & geometricData);
    
    virtual void AddControl(UIControl *control);
	virtual void RemoveControl(UIControl *control);
	virtual void RemoveAllControls();
	virtual void BringChildFront(UIControl *_control);
	virtual void BringChildBack(UIControl *_control);
	virtual void InsertChildBelow(UIControl * _control, UIControl * _belowThisChild);
	virtual void InsertChildAbove(UIControl * _control, UIControl * _aboveThisChild);
	virtual void SendChildBelow(UIControl * _control, UIControl * _belowThisChild);
	virtual void SendChildAbove(UIControl * _control, UIControl * _aboveThisChild);
    virtual const List<UIControl*> &GetChildren();
	
    bool IsScrolling();
    
protected:
	//called on TOUCH_UP which is now scroll or zoom event
	virtual void		ScrollTouch(UIEvent *currentTouch) {};

	enum 
	{
		STATE_NONE = 0,
		STATE_SCROLL,
		STATE_ZOOM,
		STATE_DECCELERATION,
		STATE_SCROLL_TO_SPECIAL,
	};
	
	
	
	void		StartScroll(Vector2 startScrollPosition);
	void		ProcessScroll(Vector2 currentScrollPosition);
	void		EndScroll();
	void		PerformScroll();
	
	Vector2		contentSize;
	
	int32		state;
	Vector2		scrollPosition; // Used only during rendering process
	Vector2		scrollOrigin;
	Vector2		scrollCurrentShift;	
	Vector2		scrollZero; //point of autoscroll aim
	// Click information
	Vector2		clickStartPosition;
	Vector2		clickEndPosition;
	// Scroll information
	Vector2		scrollStartInitialPosition;	// position of click
	Vector2		scrollStartPosition;		// position related to current scroll start pos, can be different from scrollStartInitialPosition
	Vector2		scrollCurrentPosition;	// scroll current position
	Vector2		drawScrollPos;
	bool		scrollStartMovement;
	UIEvent		scrollTouch;
	float64		scrollStartTime;
	uint64		touchStartTime;
	float32		scrollPixelsPerSecond;
	Vector2		deccelerationSpeed;
	float32		deccelerationTime;
	
	//zoom
	UIEvent		zoomTouches[2];
	Vector2		zoomStartPositions[2];
	Vector2		zoomCurrentPositions[2];
	float32		prevZoomScale;
	float32		zoomScale;
	float32		minScale;
	float32		maxScale;
	
	static const int32		MAX_MOUSE_POSITIONS = 16;
	static const uint64		TOUCH_BEGIN_MS = 250;
	
	Vector2		lastMousePositions[MAX_MOUSE_POSITIONS];
	int32		positionIndex;
	
	uint64		lastTapTime;
    
private:    
    UIControl   *contentControl;
};

#endif //__UI_ZOOM_CONTROL_H__