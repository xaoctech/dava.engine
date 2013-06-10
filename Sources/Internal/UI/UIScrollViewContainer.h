//
//  UIScrollViewContainer.h
//  Framework
//
//  Created by Denis Bespalov on 4/29/13.
//
//

#ifndef __Framework__UIScrollViewContainer__
#define __Framework__UIScrollViewContainer__

#include "DAVAEngine.h"

namespace DAVA 
{

class UIScrollViewContainer : public UIControl
{
public:
	UIScrollViewContainer(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~UIScrollViewContainer();
	
	virtual UIControl *Clone();
	virtual void CopyDataFrom(UIControl *srcControl);
	
public:
	virtual void Update(float32 timeElapsed);
	virtual void Input(UIEvent *currentTouch);
	
	virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);
	
	//called on TOUCH_UP which is now scroll or zoom event
	//virtual void		ScrollTouch(UIEvent *currentTouch) {};

	Vector2		scrollOrigin;

protected:

	void		StartScroll(Vector2 startScrollPosition);
	void		ProcessScroll(Vector2 currentScrollPosition);
	void		EndScroll();
	//void		PerformScroll();
	void		ScrollToPosition(const Vector2& position);
	void   		SaveChilds(UIControl *parent, UIYamlLoader * loader, YamlNode * parentNode);

	enum 
	{
		STATE_NONE = 0,
		STATE_SCROLL,
		STATE_ZOOM,
		STATE_DECCELERATION,
		STATE_SCROLL_TO_SPECIAL,
	};

	int32		state;
//	Vector2		scrollPosition; // Used only during rendering process

	Vector2		scrollCurrentShift;	
/*	Vector2		scrollZero; //point of autoscroll aim
	// Click information
	Vector2		clickStartPosition;
	Vector2		clickEndPosition;*/
	// Scroll information
	Vector2		scrollStartInitialPosition;	// position of click
	Vector2		scrollStartPosition;		// position related to current scroll start pos, can be different from scrollStartInitialPosition
	Vector2		scrollCurrentPosition;	// scroll current position
/*	Vector2		drawScrollPos;*/
	bool		scrollStartMovement;
	UIEvent		scrollTouch;
/*	float64		scrollStartTime;
	uint64		touchStartTime;
	float32		scrollPixelsPerSecond;
	Vector2		deccelerationSpeed;
	float32		deccelerationTime;
	
	//zoom
	UIEvent	zoomTouches[2];
	Vector2		zoomStartPositions[2];
	Vector2		zoomCurrentPositions[2];
	float32		prevZoomScale;
	float32		zoomScale;
	float32		minScale;
	float32		maxScale;
	
	static const int32		MAX_MOUSE_POSITIONS = 16;
	static const float32	SCROLL_BEGIN_PIXELS = 8.0f;
	static const uint64		TOUCH_BEGIN_MS = 250;
	
	Vector2		lastMousePositions[MAX_MOUSE_POSITIONS];
	int32			positionIndex;
	
	uint64		lastTapTime;*/
};
};

#endif /* defined(__Framework__UIScrollViewContainer__) */
