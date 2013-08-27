%module UIControl
%{
#include "UI/UIControl.h"
%}

%import Vector.i
%import Rect.i

namespace DAVA
{
	
class UIGeometricData
{
	
public:
	UIGeometricData()
	{
		cosA = 1.0f;
		sinA = 0;
		oldAngle = 0;
		angle = 0;
	}
	Vector2 position;
	Vector2 size;
	
	Vector2 pivotPoint;
	Vector2	scale;
	float32	angle;
	
	float32 cosA;
	float32 sinA;

	const Rect &GetUnrotatedRect() const
	{
		return unrotatedRect;
	}
};

class UIControl
{
public:
	enum eControlState
	{
		STATE_NORMAL			= 1 << 0,//!<Control isn't under influence of any activities.
		STATE_PRESSED_INSIDE	= 1 << 1,//!<Mouse or touch comes into control.
		STATE_PRESSED_OUTSIDE	= 1 << 2,//!<Mouse or touch comes into control but dragged outside of control.
		STATE_DISABLED			= 1 << 3,//!<Control is disabled (don't process any input). Use this state only if you want change graphical representation of the control. Don't use this state for the disabling inputs for parts of the controls hierarchy!.
		STATE_SELECTED			= 1 << 4,//!<Just a state for base control, nothing more.
		STATE_HOVER				= 1 << 5,//!<This bit is rise then mouse is over the control.
		
		STATE_COUNT				=	6
	};
	
	enum eEventType
	{
		EVENT_TOUCH_DOWN			= 1,//!<Trigger when mouse button or touch comes down inside the control.
		EVENT_TOUCH_UP_INSIDE		= 2,//!<Trigger when mouse pressure or touch processed by the control is released.
		EVENT_VALUE_CHANGED			= 3,//!<Used only with sliders for now. Trigger when value of the slider is changed.
		EVENT_HOVERED_SET           = 4,//!<
		EVENT_HOVERED_REMOVED       = 5,//!<
		EVENT_FOCUS_SET             = 6,//!<Trigger when control becomes focused
		EVENT_FOCUS_LOST            = 7,//!<Trigger when control losts focus
		EVENT_TOUCH_UP_OUTSIDE		= 8,//!<Trigger when mouse pressure or touch processed by the control is released outside of the control.
        EVENTS_COUNT
	};	
	
	friend class ControlSystem;	
public:

	UIControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	
	virtual const Rect & GetRect(bool absoluteCoordinates = false);
    
	virtual const Vector2 &GetPosition(bool absoluteCoordinates = false);
	
	virtual const Vector2 &GetSize() const;

	virtual const UIGeometricData &GetGeometricData();

	virtual bool GetVisible() const;

	virtual bool GetInputEnabled() const;

	virtual void SetInputEnabled(bool isEnabled, bool hierarchic = true);
	
	virtual bool GetDisabled() const;

	virtual bool GetSelected() const;

	const String & GetName() const;

	int32 GetTag() const;
    
	UIControl *GetParent();

protected:
	virtual ~UIControl();

};
};
