//
//  UIScrollView.cpp
//  Framework
//
//  Created by Denis Bespalov on 4/23/13.
//
//

#include "UI/UIScrollView.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"
#include "UI/UIControlSystem.h"
#include "Base/ObjectFactory.h"

namespace DAVA 
{
	REGISTER_CLASS(UIScrollView);
	
	
UIScrollView::UIScrollView(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
	:	UIControl(rect, rectInAbsoluteCoordinates)
	,	scrollContainer(NULL)
	,	scroll(NULL)
{
		InitAfterYaml();
}
		
UIScrollView::UIScrollView() :scrollContainer(NULL), scroll(NULL)
{
	InitAfterYaml();
}

void UIScrollView::InitAfterYaml()
{
	inputEnabled = TRUE;
	clipContents = TRUE;
	Rect r = GetRect();
	r.x = 0;
	r.y = 0;
	
	// InitAfterYaml might be called multiple times - check this and remove previous scroll, if yes.
	if (scrollContainer)
	{
		RemoveControl(scrollContainer);
		SafeRelease(scrollContainer);
	}

	scrollContainer = new UIControl(r);
	// Scroll container is a child of ScrollView
	UIControl::AddControl(scrollContainer);
	
	/*if (scrollBar)
	{
		RemoveControl(scrollBar);
		SafeRelease(scrollBar);
	}*/
	
//	scrollBar = new UIScrollBar(r, UIScrollBar::ORIENTATION_HORIZONTAL);
//	scrollBar->SetDelegate(this);
//	UIControl::AddControl(scrollBar);
	
	oldPos = 0;
	newPos = 0;
	
	mainTouch = -1;
	
	touchHoldSize = 15;

	lockTouch = FALSE;
	
	needRefresh = FALSE;
	
	if (scroll == NULL)
	{
		scroll = new ScrollHelper();
	}
}

UIScrollView::~UIScrollView()
{
	SafeRelease(scrollContainer);
	SafeRelease(scroll);
//	SafeRelease(scrollBar);
}

void UIScrollView::AddControl(UIControl *control)
{
	// Put new control into scroll container instead adding it as ScrollView child
	if (scrollContainer)
	{
  		scrollContainer->AddControl(control);
	}
}

void UIScrollView::SetHorizontalScrollPosition(float32 position)
{
	horizontalScrollPosition = position;
	
	if (scrollContainer)
	{
		Rect rect = GetRect();
		rect.x = (-1) * position;
		rect.y = 0;
		rect.dx += position;
		scrollContainer->SetRect(rect);
	}
}

float32 UIScrollView::GetHorizontalScrollPosition()
{
	return horizontalScrollPosition;
}

void UIScrollView::SetVerticalScrollPosition(float32 position)
{
	verticalScrollPosition = position;
}

float32 UIScrollView::GetVerticalScrollPosition()
{
	return verticalScrollPosition;
}

void UIScrollView::ScrollTo(float delta)
{
	scroll->Impulse(delta * -4.8f);
}

void UIScrollView::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/)
{
	UIControl::SetRect(rect, rectInAbsoluteCoordinates);
	// Resize scroll container
	if (scrollContainer)
	{
		scrollContainer->SetSize(Vector2(rect.dx, rect.dy));
	}
}

void UIScrollView::ScrollToElement(int32 index)
{
    float32 newScrollPos = 0.0f;
    SetScrollPosition(newScrollPos);
}
    
float32 UIScrollView::GetScrollPosition()
{
    return scroll->GetPosition();
}
    
void UIScrollView::SetScrollPosition(float32 newScrollPos)
{
    if(needRefresh)
	{
		FullRefresh();
	}
    
	scroll->SetPosition(-newScrollPos);
}
    
void UIScrollView::ResetScrollPosition()
{
	scrollContainer->relativePosition.x = 0;
	scrollContainer->relativePosition.y = 0;
	scroll->SetPosition(0);
}

void UIScrollView::FullRefresh()
{
	scrollContainer->RemoveAllControls();
	
	needRefresh = FALSE;
	
	int32 scrollAdd;
	int32 maxSize;

	scrollAdd = (int32)scrollContainer->GetRect().x;
	maxSize = (int32)GetRect().dx;
	scrollAdd = (int32)scrollContainer->GetRect().y;
	maxSize = (int32)GetRect().dy;

	scroll->SetViewSize((float32)maxSize);
	
	int sz = 0;
/*    int32 elCnt = delegate->ElementsCount(this);
    int32 index = 0;
	for (; index < elCnt; index++) 
	{
		int32 curPos = addPos + scrollAdd;
		int size = 0;
		if(orientation == ORIENTATION_HORIZONTAL)
		{
			size = delegate->CellWidth(this, index);
		}
		else 
		{
			size = delegate->CellHeight(this, index);
		}
		
		sz += size;
		if(curPos + size > -maxSize)
		{
			AddCellAtPos(delegate->CellAtIndex(this, index), addPos, size, index);
		}
		
		addPos += size;
		if(addPos + scrollAdd > maxSize * 2)
		{
			break;
		}
	}*/
 
  /*  index++;
    for (; index < elCnt; index++)
	{
		if(orientation == ORIENTATION_HORIZONTAL)
		{
			sz += delegate->CellWidth(this, index);
		}
		else 
		{
			sz += delegate->CellHeight(this, index);
		}
	}*/
    
	scroll->SetElementSize((float32)sz);
}

void UIScrollView::Refresh()
{
	needRefresh = TRUE;
}

void UIScrollView::Update(float32 timeElapsed)
{
	
	/*if(needRefresh)
	{
		FullRefresh();
	}
	
	float d = newPos - oldPos;
	oldPos = newPos;
	Rect r = scrollContainer->GetRect();

	r.x = scroll->GetPosition(d, SystemTimer::FrameDelta(), lockTouch);
	r.y = scroll->GetPosition(d, SystemTimer::FrameDelta(), lockTouch);

	scrollContainer->SetRect(r);
	
	List<UIControl*>::const_iterator it;
	Rect viewRect = GetGeometricData().GetUnrotatedRect();//GetRect(TRUE);
	const List<UIControl*> &scrollList = scrollContainer->GetChildren();
	List<UIControl*> removeList;
	
	//removing invisible elements
	for(it = scrollList.begin(); it != scrollList.end(); it++)
	{
		Rect crect = (*it)->GetGeometricData().GetUnrotatedRect();//GetRect(TRUE);

		if(crect.x + crect.dx < viewRect.x - viewRect.dx || crect.x > viewRect.x + viewRect.dx*2)
		{
			removeList.push_back(*it);
		}
		if(crect.y + crect.dy < viewRect.y - viewRect.dy || crect.y > viewRect.y + viewRect.dy*2)
		{
			removeList.push_back(*it);
		}
	}
	for(it = removeList.begin(); it != removeList.end(); it++)
	{
		scrollContainer->RemoveControl((*it));
	}
	
    if (!scrollList.empty()) 
    {
        //adding elements at the list end
        int32 ind = -1;
   /*     UIListCell *fc = NULL;
        for(it = scrollList.begin(); it != scrollList.end(); it++)
        {
            UIListCell *lc = (UIListCell *)(*it);
            int32 i = lc->GetIndex();
            if(i > ind)
            {
                ind = i;
                fc = lc;
            }
        }
        if(fc)
        {
            int32 borderPos;
            int32 rPos;
            int size = 0;
            int32 off;
            if(orientation == ORIENTATION_HORIZONTAL)
            {
                borderPos = (int32)(viewRect.dx + viewRect.dx / 2.0f);
                off = (int32)scrollContainer->GetRect().x;
                rPos = (int32)(fc->GetRect().x + fc->GetRect().dx + off);
            }
            else 
            {
                borderPos = (int32)(viewRect.dy + viewRect.dy / 22.0f);
                off = (int32)scrollContainer->GetRect().y;
                rPos = (int32)(fc->GetRect().y + fc->GetRect().dy + off);
            }

			int32 elementsCount = delegate->ElementsCount(this);
            while(rPos < borderPos && fc->GetIndex() < elementsCount - 1)
            {
                int32 i = fc->GetIndex() + 1;
                fc = delegate->CellAtIndex(this, i);
                if(orientation == ORIENTATION_HORIZONTAL)
                {
                    size = delegate->CellWidth(this, i);
                }
                else 
                {
                    size = delegate->CellHeight(this, i);
                }
                AddCellAtPos(fc, rPos - off, size, i);
                rPos += size;
                    //			scroll->SetElementSize((float32)(rPos - off));
            }
        }*/
        
            //adding elements at the list begin
 //       ind = maximumElementsCount;
  /*      fc = NULL;
        for(it = scrollList.begin(); it != scrollList.end(); it++)
        {
            UIListCell *lc = (UIListCell *)(*it);
            int32 i = lc->GetIndex();
            if(i < ind)
            {
                ind = i;
                fc = lc;
            }
        }
        if(fc)
        {
            int32 borderPos;
            int32 rPos;
            int size = 0;
            int32 off;
            if(orientation == ORIENTATION_HORIZONTAL)
            {
                borderPos = (int32)(-viewRect.dx/2.0f);
                off = (int32)scrollContainer->GetRect().x;
                rPos = (int32)(fc->GetRect().x + off);
            }
            else 
            {
                borderPos = (int32)(-viewRect.dy/2.0f);
                off = (int32)scrollContainer->GetRect().y;
                rPos = (int32)(fc->GetRect().y + off);
            }
            while(rPos > borderPos && fc->GetIndex() > 0)
            {
                int32 i = fc->GetIndex() - 1;
                fc = delegate->CellAtIndex(this, i);
                if(orientation == ORIENTATION_HORIZONTAL)
                {
                    size = delegate->CellWidth(this, i);
                }
                else 
                {
                    size = delegate->CellHeight(this, i);
                }
                rPos -= size;
                AddCellAtPos(fc, rPos - off, size, i);
            }
        }*/
    //}
    //else
    {
    //    FullRefresh();
    }


	
}

void UIScrollView::Draw(const UIGeometricData &geometricData)
{
	if(needRefresh)
	{
		FullRefresh();
	}
	UIControl::Draw(geometricData);
}

void UIScrollView::Input(UIEvent *currentInput)
{
	newPos = currentInput->point.x;
	newPos = currentInput->point.y;

	switch (currentInput->phase) 
	{
		case UIEvent::PHASE_BEGAN:
		{
			lockTouch = TRUE;
			oldPos = newPos;
			mainTouch = currentInput->tid;
		}
			break;
		case UIEvent::PHASE_DRAG:
		{
		}
			break;
		case UIEvent::PHASE_ENDED:
		{
			lockTouch = FALSE;
			mainTouch = -1;
		}
			break;
	}
}

bool UIScrollView::SystemInput(UIEvent *currentInput)
{
	if(!inputEnabled || !visible || controlState & STATE_DISABLED)
	{
		return false;
	}

	if(currentInput->touchLocker != this)
	{
		if(currentInput->phase == UIEvent::PHASE_BEGAN)
		{
			if(IsPointInside(currentInput->point))
			{
				PerformEvent(EVENT_TOUCH_DOWN);
				Input(currentInput);
			}
		}
		else if(currentInput->tid == mainTouch && currentInput->phase == UIEvent::PHASE_DRAG)
		{
			//if(orientation == ORIENTATION_HORIZONTAL)
			{
				if(abs(currentInput->point.x - newPos) > touchHoldSize)
				{
					UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
					newPos = currentInput->point.x;
					return TRUE;
				}
			/*}
			else
			{*/
				if(abs(currentInput->point.y - newPos) > touchHoldSize)
				{
					UIControlSystem::Instance()->SwitchInputToControl(mainTouch, this);
					newPos = currentInput->point.y;
					return TRUE;
				}
			}
		}
		else if(currentInput->tid == mainTouch && currentInput->phase == UIEvent::PHASE_ENDED)
		{
			mainTouch = -1;
			lockTouch = false;
		}

		

	}

	return UIControl::SystemInput(currentInput);
}
	
void UIScrollView::OnSelectEvent(BaseObject *pCaller, void *pUserData, void *callerData)
{
/*	if(delegate)
	{
		delegate->OnCellSelected(this, (UIListCell*)pCaller);
	}*/
}
	
void UIScrollView::SetTouchHoldDelta(int32 holdDelta)
{
	touchHoldSize = holdDelta;
}
int32 UIScrollView::GetTouchHoldDelta()
{
	return touchHoldSize;
}

void UIScrollView::SetSlowDownTime(float newValue)
{
	scroll->SetSlowDownTime(newValue);
}
void UIScrollView::SetBorderMoveModifer(float newValue)
{
	scroll->SetBorderMoveModifer(newValue);
}

void UIScrollView::SystemWillAppear()
{
	UIControl::SystemWillAppear();	
	Refresh();
}

void UIScrollView::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
		
/*	YamlNode * orientNode = node->Get("orientation");
	if (orientNode)
	{
		if (orientNode->AsString() == "ORIENTATION_VERTICAL")
			orientation = ORIENTATION_VERTICAL;
		else if (orientNode->AsString() == "ORIENTATION_HORIZONTAL")
			orientation = ORIENTATION_HORIZONTAL;
		else 
		{
			DVASSERT(0 && "Orientation constant is wrong"); 
		}
	}
	// Load aggregator path
	YamlNode * aggregatorPathNode = node->Get("aggregatorPath");
	if (aggregatorPathNode)
	{
		aggregatorPath = aggregatorPathNode->AsString();
	}*/
	
		// TODO
	InitAfterYaml();
}

UIControl *UIScrollView::Clone()
{
	UIScrollView *c = new UIScrollView(GetRect());
	c->CopyDataFrom(this);
	return c;
}

YamlNode * UIScrollView::SaveToYamlNode(UIYamlLoader * loader)
{
	YamlNode *node = UIControl::SaveToYamlNode(loader);
	//Temp variable
	String stringValue;
    
	//Control Type
	SetPreferredNodeType(node, "UIScrollView");

	return node;
}

float32 UIScrollView::VisibleAreaSize(UIScrollBar *forScrollBar)
{
    return scroll->GetViewSize();
}
    
float32 UIScrollView::TotalAreaSize(UIScrollBar *forScrollBar)
{
    return scroll->GetElementSize();
}
    
float32 UIScrollView::ViewPosition(UIScrollBar *forScrollBar)
{
    return scroll->GetPosition();
}
    
void UIScrollView::OnViewPositionChanged(UIScrollBar *byScrollBar, float32 newPosition)
{
    scroll->SetPosition(-newPosition);
}
    
List<UIControl* >& UIScrollView::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(scrollContainer);
	return realChildren;
}

};