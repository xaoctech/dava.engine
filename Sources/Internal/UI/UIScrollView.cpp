/*
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
    * Created by Vitaliy Borodovsky
*/


#include "UIScrollView.h"
#include "Base/ObjectFactory.h"
#include "UI/UIScrollViewContainer.h"

namespace DAVA 
{
	
REGISTER_CLASS(UIScrollView);

UIScrollView::UIScrollView(const Rect &rect, const Vector2 &_contentSize, bool rectInAbsoluteCoordinates/* = false*/)
:	UIControl(rect, rectInAbsoluteCoordinates),
	contentSize(_contentSize),
	state(STATE_NONE),
	positionIndex(0),
	zoomScale(1.0f),
	scrollOrigin(0, 0),
	scrollZero(0, 0),
	lastTapTime(0),
	touchStartTime(0),
	scrollContainer(NULL)
{
	inputEnabled = true;
	multiInput = true;
	SetClipContents(true);
	SetDebugDraw(true, true);
	
	Rect r = GetRect();
	r.x = 0;
	r.y = 0;
	
	// InitAfterYaml might be called multiple times - check this and remove previous scroll, if yes.
	if (scrollContainer)
	{
		RemoveControl(scrollContainer);
		SafeRelease(scrollContainer);
	}

	scrollContainer = new UIScrollViewContainer(r);
	scrollContainer->SetInputEnabled(true);
	scrollContainer->SetMultiInput(true);
	scrollContainer->SetDebugDraw(true);
	scrollContainer->SetDebugDrawColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
	scrollContainer->SetName("SCROLL CONTAINER");
//	scrollContainer->GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
//	scrollContainer->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
//	scrollContainer->SetClipContents(true);
	// Scroll container is a child of ScrollView
	UIControl::AddControl(scrollContainer);
	
	prevZoomScale = minScale = maxScale = zoomScale;
}

UIScrollView::~UIScrollView()
{
	SafeRelease(scrollContainer);
}

void UIScrollView::AddControl(UIControl *control)
{
//	UIControl::AddControl(control);
	// Put new control into scroll container instead adding it as ScrollView child
	if (scrollContainer)
	{
 		scrollContainer->AddControl(control);
		
		//Rect rect = scrollContainer->GetRect();
		Rect parentRect = this->GetRect();
		// Initial content max size is actual control sizes
		float32 maxSizeX = parentRect.dx;
		float32 maxSizeY = parentRect.dy;

		List<UIControl*> childslist = scrollContainer->GetChildren();
		for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
		{
       		UIControl *childControl = (UIControl*)(*it);
	   		if (!childControl)
	   			continue;
		
			Rect childRect = childControl->GetRect();
			// Calculate control full "length" and "height"
			float32 controlSizeX = childRect.x + childRect.dx;
			float32 controlSizeY = childRect.y + childRect.dy;
			// Check horizontal size
			if (controlSizeX >= maxSizeX)
			{
				maxSizeX = controlSizeX;
			}
			if (controlSizeY >= maxSizeY)
			{
				maxSizeY = controlSizeY;
			}
		}
		// Update scroll view content size
		scrollContainer->SetRect(Rect(10, 10, maxSizeX, maxSizeY));
	}	
}

    
List<UIControl* >& UIScrollView::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(scrollContainer);
	return realChildren;
}

UIControl* UIScrollView::Clone()
{
	UIScrollView *t = new UIScrollView(GetRect());
	t->CopyDataFrom(this);
	return t;
}
	
void UIScrollView::CopyDataFrom(UIControl *srcControl)
{
	UIControl::CopyDataFrom(srcControl);
	UIScrollView* t = (UIScrollView*) srcControl;
	
	scrollOrigin = t->scrollOrigin;
	contentSize = t->contentSize;
}

void UIScrollView::SetOffset(const Vector2 & offset)
{
	float32 offsetX = (-1) * offset.x;
	float32 offsetY = (-1) * offset.y;
	Rect rect = this->GetRect();
	
	// Apply scroll offset only if it value don't exceed content size
	if ((offsetX + rect.dx) <= contentSize.x)
	{
		scrollOrigin.x = offset.x;
	}
	
	if ((offsetY + rect.dy) <= contentSize.y)
	{
		scrollOrigin.y = offset.y;
	}
}

const Vector2& UIScrollView::GetOffset() const
{
	return scrollOrigin;
}

void UIScrollView::SetScales(float32 _minScale, float32 _maxScale)
{
	minScale = _minScale;
	maxScale = _maxScale;
}

void UIScrollView::SetScale(float currentScale)
{ 
	zoomScale = currentScale;
	
	scrollZero = Vector2(0.f, 0.f);
	
	float32 adjW = contentSize.dx*zoomScale;
	float32 adjH = contentSize.dy*zoomScale;
	
	if(adjW < size.x)
	{
		scrollZero.x = (size.x - adjW)/2.f;
	}
	
	if(adjH < size.y)
	{
		scrollZero.y = (size.y - adjH)/2.f;
	}
}
	
void UIScrollView::PerformScroll()
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
}

void UIScrollView::Update(float32 timeElapsed)
{
	UIControl::Update(timeElapsed);

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
		if((scrollOrigin.x > 0) && ((scrollOrigin.x + contentSize.dx * zoomScale) < (size.x)))
		{
			float32 delta = scrollOrigin.x-scrollZero.x;
			scrollOrigin.x -= delta * timeElapsed * 5;
		}
		else if(scrollOrigin.x > 0)
		{
			scrollOrigin.x -= (scrollOrigin.x) * timeElapsed * 5;
		}
		else if((scrollOrigin.x + contentSize.dx * zoomScale) < (size.x))
		{
			scrollOrigin.x += ((size.x) - (scrollOrigin.x + contentSize.dx * zoomScale)) * timeElapsed * 5;
		}
		
		// vcenter
		if((scrollOrigin.y > 0) && ((scrollOrigin.y + contentSize.dy * zoomScale) < (size.y)))
		{
			float32 delta = scrollOrigin.y-scrollZero.y;
			scrollOrigin.y -= delta * timeElapsed * 5;
		}
		else if(scrollOrigin.y > 0)
		{
			scrollOrigin.y -= (scrollOrigin.y) * timeElapsed * 5;
		}
		else if((scrollOrigin.y + contentSize.dy * zoomScale) < (size.y))
		{
			scrollOrigin.y += ((size.y) - (scrollOrigin.y + contentSize.dy * zoomScale)) * timeElapsed * 5;
		}
	} 
	
	//scrolling over the edge
	drawScrollPos.x = scrollCurrentShift.x;
	drawScrollPos.y = scrollCurrentShift.y;

	RecalculateContentSize();*/
}

void UIScrollView::StartScroll(Vector2 _startScrollPosition)
{
	scrollStartInitialPosition = _startScrollPosition;
	scrollStartPosition = _startScrollPosition;
	scrollCurrentPosition = _startScrollPosition;
	scrollStartMovement = false;
	
	lastMousePositions[positionIndex & (MAX_MOUSE_POSITIONS - 1)] = _startScrollPosition;
	positionIndex++;
	positionIndex &= (MAX_MOUSE_POSITIONS - 1);
}

void UIScrollView::ProcessScroll(Vector2 _currentScrollPosition)
{
	scrollCurrentPosition = _currentScrollPosition;
	
	//	This check is required on iPhone, to avoid bugs in movement.
	
#if defined(__DAVAENGINE_IPHONE__)
	Vector2 lineLenght = scrollCurrentPosition - scrollStartInitialPosition;

	if (lineLenght.Length() >= SCROLL_BEGIN_PIXELS)
#endif
	{
		touchStartTime = 0;
		if (!scrollStartMovement)scrollStartPosition = scrollCurrentPosition;
		scrollCurrentShift = Vector2((scrollCurrentPosition.x - scrollStartPosition.x),  (scrollCurrentPosition.y - scrollStartPosition.y));
		scrollStartMovement = true;
	}

	lastMousePositions[positionIndex & (MAX_MOUSE_POSITIONS - 1)] = _currentScrollPosition;
	positionIndex++;
	positionIndex &= (MAX_MOUSE_POSITIONS - 1);
}

void UIScrollView::EndScroll()
{
	scrollOrigin.x += scrollCurrentShift.x;
	scrollOrigin.y += scrollCurrentShift.y;

	scrollCurrentShift.x = 0;
	scrollCurrentShift.y = 0;
}

bool UIScrollView::SystemInput(UIEvent *currentTouch)
{
	if(!inputEnabled || !visible || controlState & STATE_DISABLED)
	{
		return false;
	}

	if(currentTouch->touchLocker != this)
	{
		if(clipContents && currentTouch->phase == UIEvent::PHASE_BEGAN)
		{
			if(!GetRect(TRUE).PointInside(currentTouch->point))
			{
				return FALSE;
			}
		}
		
		/*for(List<UIControl*>::iterator it = childs.begin(); it != childs.end(); ++it)
        {
			if((*it)->GetRect(TRUE).PointInside(currentTouch->point))
			{
				return (*it)->SystemInput(currentTouch);
			}
		}*/
		
		//return scrollContainer->SystemInput(currentTouch);
	}

	return UIControl::SystemInput(currentTouch);
}

void UIScrollView::Input(UIEvent *currentTouch)
{
	UIControl::Input(currentTouch);
	
/*	Vector<UIEvent> touches = UIControlSystem::Instance()->GetAllInputs();

	if(1 == touches.size())
	{*/
       /* bool spaceIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SPACE);
       // bool spaceIsPressed = true;
        if(!spaceIsPressed)
        {
            for(List<UIControl*>::iterator it = childs.begin(); it != childs.end(); ++it)
            {
                (*it)->Input(currentTouch);
            }

            return;
        }*/
        
	/*	switch(currentTouch->phase)
		{
			case UIEvent::PHASE_BEGAN:
			{
				scrollTouch = *currentTouch;
				
				Vector2 start = currentTouch->point;
				StartScroll(start);

				// init scrolling speed parameters
				scrollPixelsPerSecond = 0.0f;
				scrollStartTime = currentTouch->timestamp;//to avoid cast from uint64 to float64 SystemTimer::Instance()->AbsoluteMS();
				touchStartTime = SystemTimer::Instance()->AbsoluteMS();
				state = STATE_SCROLL;

				clickStartPosition = start;
			}
			break;
			case UIEvent::PHASE_DRAG:
			{
				if(state == STATE_SCROLL) 
				{
					if(currentTouch->tid == scrollTouch.tid)
					{
						// scrolling speed get parameters
						float64 scrollCurrentTime = currentTouch->timestamp;
						Vector2 scrollPrevPosition = scrollCurrentPosition;
						
						// perform scrolling
						ProcessScroll(currentTouch->point);
						
						// calculate scrolling speed
						float64 tmp = scrollCurrentTime;
						if(fabs(scrollCurrentTime - scrollStartTime) < 1e-9)
						{
							tmp += .1f;
						}
						
						Vector2 lineLength = scrollCurrentPosition - scrollPrevPosition;
						scrollPixelsPerSecond = (float32)((float64)lineLength.Length() / (tmp - scrollStartTime));
						scrollStartTime = scrollCurrentTime;
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
						Vector2 scrollPrevPos = lastMousePositions[(positionIndex - 3) & (MAX_MOUSE_POSITIONS - 1)];
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
						
						EndScroll();
						
						//scrollTouch = 0;
						if(scrollStartMovement)
						{
							state = STATE_DECCELERATION;
						}
						
						clickEndPosition = currentTouchPos;
					}
					
					if(touchStartTime)
					{
						touchStartTime = 0;
						PerformScroll();
					}
				}
			}
				break;
		}
	}*/
	/*else if(2 == touches.size())
	{
		switch(currentTouch->phase) 
		{
			case UIEvent::PHASE_BEGAN:
			{
				if (state == STATE_SCROLL)
				{
					EndScroll();
					//scrollTouch = 0;
				}
				
				// init zoom parameters
				state = STATE_ZOOM;
				
				zoomTouches[0] = touches[0];
				zoomTouches[1] = touches[1];
				
				zoomStartPositions[0] = zoomTouches[0].point;
				zoomStartPositions[1] = zoomTouches[1].point;
				
				prevZoomScale = zoomScale; // save current scale to perform scaling in zoom mode
			}
			break;
			case UIEvent::PHASE_DRAG:
				if(state == STATE_ZOOM)
				{
					zoomTouches[0] = touches[0];
					zoomTouches[1] = touches[1];
					
					zoomCurrentPositions[0] = zoomTouches[0].point;
					zoomCurrentPositions[1] = zoomTouches[1].point;
					
					float initialDistance = sqrtf(
									  (zoomStartPositions[0].x - zoomStartPositions[1].x) * 
									  (zoomStartPositions[0].x - zoomStartPositions[1].x) + 
									  
									  (zoomStartPositions[0].y - zoomStartPositions[1].y) * 
									  (zoomStartPositions[0].y - zoomStartPositions[1].y));
					
					float currentDistance = sqrtf(
									  (zoomCurrentPositions[0].x - zoomCurrentPositions[1].x) * 
									  (zoomCurrentPositions[0].x - zoomCurrentPositions[1].x) + 
									  
									  (zoomCurrentPositions[0].y - zoomCurrentPositions[1].y) * 
									  (zoomCurrentPositions[0].y - zoomCurrentPositions[1].y));
					
					float32 saveZoomScale = zoomScale;
					float32 changeCoeff = initialDistance/currentDistance;
					float32 newScale = prevZoomScale * ((1.f - changeCoeff)/2.5f + 1.f);
					//float32 changeCoeff = currentDistance/initialDistance;
					//float32 newScale = prevZoomScale * changeCoeff * changeCoeff;
					if(newScale > maxScale)newScale = maxScale;
					if(newScale < minScale)newScale = minScale; 
					
					SetScale(newScale);
					
					Vector2 center = Vector2((zoomStartPositions[0].x + zoomStartPositions[1].x) / 2,
									(zoomStartPositions[0].y + zoomStartPositions[1].y) / 2);
					
					Vector2 scaleVectorOriginal = Vector2(center.x - scrollOrigin.x, center.y - scrollOrigin.y);
					Vector2 scaleVectorNew = scaleVectorOriginal;
					scaleVectorNew.x *= zoomScale / saveZoomScale;
					scaleVectorNew.y *= zoomScale / saveZoomScale;
					
					scaleVectorNew.x -= scaleVectorOriginal.x;
					scaleVectorNew.y -= scaleVectorOriginal.y;
					
					scrollOrigin.x -= scaleVectorNew.x;
					scrollOrigin.y -= scaleVectorNew.y;
				}
			break;
			case UIEvent::PHASE_ENDED:
			{
				Vector<UIEvent>::iterator it = touches.begin();
				bool zoomToScroll = false;
				for(; it != touches.end(); ++it)
				{
					if((*it).phase == UIEvent::PHASE_DRAG)
					{
						zoomToScroll = true;
						scrollTouch = *it;
					}
				}
				
				if(zoomToScroll)
				{
					Vector2 start = scrollTouch.point;
					StartScroll(start);
					state = STATE_SCROLL;
				}
				else
				{
					if (state != STATE_DECCELERATION) 
					{
						state = STATE_NONE;
					}
				}
			}
			break;
		}
	}*/
}

void UIScrollView::SystemDraw(const UIGeometricData & geometricData)
{
	UIControl::SystemDraw(geometricData);
	/*
	float32 invScale = 1.0f;// / zoomScale;
	Vector2 drawPos;
	drawPos.x = (scrollOrigin.x + drawScrollPos.x) * invScale;
	drawPos.y = (scrollOrigin.y + drawScrollPos.y) * invScale;
	
	UIGeometricData drawData;
	drawData.position = relativePosition;
	drawData.size = size;
	drawData.pivotPoint = pivotPoint;
	drawData.scale = scale;
	drawData.angle = angle;
	drawData.AddToGeometricData(geometricData);
	
	if(clipContents)
	{//WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(drawData.GetUnrotatedRect());
	}	
	
	if (debugDrawEnabled)
	{//TODO: Add debug draw for rotated controls
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		RenderHelper::Instance()->DrawRect(drawData.GetUnrotatedRect());
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
	drawData.position = drawPos + relativePosition;
	drawData.size = size;
	drawData.pivotPoint = pivotPoint;

	drawData.scale = Vector2(zoomScale, zoomScale);
	drawData.angle = angle;
	drawData.AddToGeometricData(geometricData);
	
	if(visible)
	{
		Draw(drawData);
	}

	List<UIControl*>::iterator it = childs.begin();
	for(; it != childs.end(); it++)
	{
		(*it)->SystemDraw(drawData);
	}
	
	if(visible)
	{
		DrawAfterChilds(drawData);
	}
	if(clipContents)
	{
		RenderManager::Instance()->ClipPop();
	}*/
}
		
void UIScrollView::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	// Load size of contents
	YamlNode *contentSizeNode = node->Get("contentSize");
	if (contentSizeNode)
	{
		SetContentSize(contentSizeNode->AsPoint());
	}
	// Load curent scroll positions
	YamlNode * scrollOriginNode = node->Get("scrollOrigin");
	if (scrollOriginNode)
	{
		SetOffset(scrollOriginNode->AsPoint());
	}
}

YamlNode * UIScrollView::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    VariantType *nodeValue = new VariantType();
	
    // Control Type
	SetPreferredNodeType(node, "UIScrollView");

	// The size of contents
	Vector2 content(contentSize.x, contentSize.y);
	node->Set("contentSize", content);
	
	// Current scroll positions
	Vector2 origin(scrollOrigin.x, scrollOrigin.y);
	node->Set("scrollOrigin", origin);
	
	SafeDelete(nodeValue);
    
    return node;
}


void UIScrollView::RecalculateContentSize()
{
	float32 maxSizeX = this->GetRect().dx;
	float32 maxSizeY = this->GetRect().dy;

	Rect realContentRect = scrollContainer->GetRect();
	
	if (realContentRect.dx > maxSizeX)
	{
		maxSizeX = realContentRect.dx;
	}
	
	if (realContentRect.dy > maxSizeY)
	{
		maxSizeY = realContentRect.dy;
	}
	
	SetContentSize(Vector2(maxSizeX, maxSizeY));
	
	
/*	float32 currentContentSizeX = contentSize.x;
	float32 currentContentSizeY = contentSize.y;	
	// Initial content max size is actual control sizes
	float32 maxSizeX = this->GetRect().dx;
	float32 maxSizeY = this->GetRect().dy;

	List<UIControl*> childslist = this->GetChildren();
	for(List<UIControl*>::iterator it = childslist.begin(); it != childslist.end(); ++it)
    {
       	UIControl *childControl = (UIControl*)(*it);
	   	if (!childControl)
	   		continue;
		
		Rect childRect = childControl->GetRect();
		// Calculate control full "length" and "height"
		float32 controlSizeX = childRect.x + childRect.dx;
		float32 controlSizeY = childRect.y + childRect.dy;
		// Check horizontal size
		if (controlSizeX >= currentContentSizeX)
		{
			if (controlSizeX >= maxSizeX)
			{
				maxSizeX = controlSizeX;
			}
		}
		// Check vertical size
		if (controlSizeY >= currentContentSizeY)
		{
			if (controlSizeY >= maxSizeY)
			{
				maxSizeY = controlSizeY;
			}
		} 		
	}
	// Update scroll view content size
	SetContentSize(Vector2(maxSizeX, maxSizeY));*/
}

}
