//
//  UIScrollViewContainer.cpp
//  Framework
//
//  Created by Denis Bespalov on 4/29/13.
//
//

#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIScrollViewContainer);


UIScrollViewContainer::UIScrollViewContainer(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	scrollOrigin(0, 0)
{
	this->SetInputEnabled(true);
	this->SetMultiInput(true);
	this->SetDebugDraw(true);
//	this->SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
//	this->GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
//	this->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

UIScrollViewContainer::~UIScrollViewContainer()
{
}

UIControl* UIScrollViewContainer::Clone()
{
	UIScrollViewContainer *t = new UIScrollViewContainer(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollViewContainer::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	
	UIScrollViewContainer* t = (UIScrollViewContainer*) srcControl;
		
	scrollOrigin = t->scrollOrigin;
}

void UIScrollViewContainer::StartScroll(Vector2 _startScrollPosition)
{
	scrollStartInitialPosition = _startScrollPosition;
	scrollStartPosition = _startScrollPosition;
	scrollCurrentPosition = _startScrollPosition;
	scrollStartMovement = false;
}

void UIScrollViewContainer::ProcessScroll(Vector2 _currentScrollPosition)
{
	scrollCurrentPosition = _currentScrollPosition;

	//	This check is required on iPhone, to avoid bugs in movement.	
#if defined(__DAVAENGINE_IPHONE__)
	Vector2 lineLenght = scrollCurrentPosition - scrollStartInitialPosition;

	if (lineLenght.Length() >= SCROLL_BEGIN_PIXELS)
#endif
	{
		//touchStartTime = 0;
		if (!scrollStartMovement) scrollStartPosition = scrollCurrentPosition;
		scrollCurrentShift = Vector2((scrollCurrentPosition.x - scrollStartPosition.x),  (scrollCurrentPosition.y - scrollStartPosition.y));
		scrollStartMovement = true;
	}

	ScrollToPosition(scrollCurrentShift);
}

void UIScrollViewContainer::EndScroll()
{
	if (scrollStartMovement)
	{
		scrollOrigin.x = scrollCurrentShift.x;
		scrollOrigin.y = scrollCurrentShift.y;
	}

	scrollCurrentShift.x = 0;
	scrollCurrentShift.y = 0;
}

/*void UIScrollViewContainer::PerformScroll()
{
	Vector2 clickEndPosition = clickStartPosition;
	clickEndPosition.x -= (scrollOrigin.x);
	clickEndPosition.y -= (scrollOrigin.y);
	
	clickEndPosition.x /= zoomScale;
	clickEndPosition.y /= zoomScale;
	
	UIEvent modifiedTouch = scrollTouch;
	modifiedTouch.phase = UIEvent::PHASE_ENDED;
	modifiedTouch.point = clickEndPosition;
	
	ScrollTouch(&modifiedTouch);
}*/

void UIScrollViewContainer::ScrollToPosition(const DAVA::Vector2 &position)
{
	Rect rect = this->GetRect();	
	rect.x = scrollOrigin.x + position.x;
	rect.y = scrollOrigin.y + position.y;	
	this->SetRect(rect);
}

void UIScrollViewContainer::Input(UIEvent *currentTouch)
{
	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();
	
	if(1 == touches.size())
	{
		switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				
				Vector2 start = currentTouch->point;
				StartScroll(start);

				// init scrolling speed parameters
				//scrollPixelsPerSecond = 0.0f;
				//scrollStartTime = currentTouch->timestamp;//to avoid cast from uint64 to float64 SystemTimer::Instance()->AbsoluteMS();
				//touchStartTime = SystemTimer::Instance()->AbsoluteMS();
				state = STATE_SCROLL;

				//clickStartPosition = start;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						// scrolling speed get parameters
						//float64 scrollCurrentTime = currentTouch->timestamp;
						//Vector2 scrollPrevPosition = scrollCurrentPosition;
						
						// perform scrolling
						ProcessScroll(currentTouch->point);
						
						// calculate scrolling speed
						/*float64 tmp = scrollCurrentTime;
						if(fabs(scrollCurrentTime - scrollStartTime) < 1e-9)
						{
							tmp += .1f;
						}
						
						Vector2 lineLength = scrollCurrentPosition - scrollPrevPosition;
						scrollPixelsPerSecond = (float32)((float64)lineLength.Length() / (tmp - scrollStartTime));
						scrollStartTime = scrollCurrentTime;*/
					}
				}
			}
				break;
			case UIEvent::PHASE_ENDED:
			{
				if(state == STATE_SCROLL)
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						/*Vector2 scrollPrevPos = lastMousePositions[(positionIndex - 3) & (MAX_MOUSE_POSITIONS - 1)];
						Vector2 currentTouchPos = currentTouch->point; 
						
						deccelerationSpeed = Vector2(currentTouchPos.x, currentTouchPos.y);
						deccelerationSpeed.x -= scrollPrevPos.x;
						deccelerationSpeed.y -= scrollPrevPos.y;
						
						float32 deccelerationSpeedLen = sqrtf(deccelerationSpeed.x * deccelerationSpeed.x + deccelerationSpeed.y * deccelerationSpeed.y);
						if (deccelerationSpeedLen >= 0.00001f)
						{
							deccelerationSpeed.x /= deccelerationSpeedLen;
							deccelerationSpeed.y /= deccelerationSpeedLen;
						}
						else
						{
							deccelerationSpeed.x = 0.0f;
							deccelerationSpeed.y = 0.0f;
						}
						*/
						EndScroll();
						
						//scrollTouch = 0;
						//if(scrollStartMovement)
						{
							state = STATE_DECCELERATION;
						}
						
						//clickEndPosition = currentTouchPos;
					}
					
					/*if(touchStartTime)
					{
						touchStartTime = 0;
						PerformScroll();
					}*/
				}
			}
				break;
		}
	}
}

void UIScrollViewContainer::Update(float32 timeElapsed)
{
	if (state == STATE_NONE)
	{
		return;
	}

	if(state == STATE_DECCELERATION)
	{
		Rect contentRect = this->GetRect();
		scrollOrigin = Vector2(contentRect.x, contentRect.y);
	}
	
	if(state != STATE_ZOOM && state != STATE_SCROLL) 
	{
		Rect contentRect = this->GetRect();
		Rect parentRect = this->GetParent()->GetRect();
	
		float32 shiftSizeX = abs(contentRect.x) + parentRect.dx;
		float32 shiftSizeY = abs(contentRect.y) + parentRect.dy;
	
		if (contentRect.x > 0)
		{
			contentRect.x -= 1;
			this->SetRect(contentRect);
		}
		else if (shiftSizeX > contentRect.dx)
		{
			contentRect.x += 1;
			this->SetRect(contentRect);
		}
	
		if (contentRect.y > 0)
		{
			contentRect.y -=  1;
			this->SetRect(contentRect);
		}
		else if (shiftSizeY > contentRect.dy)
		{
			contentRect.y += 1;
			this->SetRect(contentRect);
		}
	}
	
/*	if(touchStartTime)
	{
		uint64 delta = SystemTimer::Instance()->AbsoluteMS() - touchStartTime;
		if(delta > TOUCH_BEGIN_MS)
		{
			touchStartTime = 0;
		
			PerformScroll();
		}
	}
	
	if(state == STATE_DECCELERATION)
	{
		float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
		
		scrollOrigin.x += deccelerationSpeed.x * scrollPixelsPerSecond * timeElapsed;
		scrollOrigin.y += deccelerationSpeed.y * scrollPixelsPerSecond * timeElapsed;
		
		scrollPixelsPerSecond *= 0.7f;
		
		if (scrollPixelsPerSecond <= 0.1f)
		{
			scrollPixelsPerSecond = 0.f;
			state = STATE_NONE;
		}
	}
	
	if(state != STATE_ZOOM && state != STATE_SCROLL)
	{
		// hcenter
		if((scrollOrigin.x > 0) && ((scrollOrigin.x + GetRect().dx * zoomScale) < (size.x)))
		{
			float32 delta = scrollOrigin.x-scrollZero.x;
			scrollOrigin.x -= delta * timeElapsed * 5;
		}
		else if(scrollOrigin.x > 0)
		{
			scrollOrigin.x -= (scrollOrigin.x) * timeElapsed * 5;
		}
		else if((scrollOrigin.x + GetRect().dx * zoomScale) < (size.x))
		{
			scrollOrigin.x += ((size.x) - (scrollOrigin.x + GetRect().dx * zoomScale)) * timeElapsed * 5;
		}
		
		// vcenter
		if((scrollOrigin.y > 0) && ((scrollOrigin.y + GetRect().dy * zoomScale) < (size.y)))
		{
			float32 delta = scrollOrigin.y-scrollZero.y;
			scrollOrigin.y -= delta * timeElapsed * 5;
		}
		else if(scrollOrigin.y > 0)
		{
			scrollOrigin.y -= (scrollOrigin.y) * timeElapsed * 5;
		}
		else if((scrollOrigin.y + GetRect().dy * zoomScale) < (size.y))
		{
			scrollOrigin.y += ((size.y) - (scrollOrigin.y + GetRect().dy * zoomScale)) * timeElapsed * 5;
		}		
	}

	//scrolling over the edge
	drawScrollPos.x = scrollCurrentShift.x;
	drawScrollPos.y = scrollCurrentShift.y;*/
}

/*void UIScrollViewContainer::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	// Load size of contents
//	YamlNode *contentSizeNode = node->Get("contentSize");
//	if (contentSizeNode)
	{
//		SetContentSize(contentSizeNode->AsPoint());
	}
	// Load curent scroll positions
//	YamlNode * scrollOriginNode = node->Get("scrollOrigin");
//	if (scrollOriginNode)
//	{
	//	SetOffset(scrollOriginNode->AsPoint());
//	}
}*/

YamlNode * UIScrollViewContainer::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
	
    // Control Type
	SetPreferredNodeType(node, "UIScrollViewContainer");
	// Save scroll view container childs including all sub-childs
	SaveChilds(this, loader, node);
    
    return node;
}

void UIScrollViewContainer::SaveChilds(UIControl *parent, UIYamlLoader * loader, YamlNode * parentNode)
{
	List<UIControl*> childslist = parent->GetRealChildren();
	for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
    {
       	UIControl *childControl = (UIControl*)(*it);
	   	if (!childControl)
	   		continue;

		YamlNode* childNode = childControl->SaveToYamlNode(loader);		
		parentNode->AddNodeToMap(childControl->GetName(), childNode);
		// Save sub-childs
		SaveChilds(childControl, loader, childNode);
	}
}

};