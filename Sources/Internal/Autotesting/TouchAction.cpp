#include "Autotesting/TouchAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

#include "Core/Core.h"

namespace DAVA
{

TouchAction::TouchAction(int32 _id) : Action()
    , id(_id)
{
}

TouchAction::~TouchAction()
{
}

void TouchAction::TouchDown(const Vector2 &point)
{
    //TODO: multitouch

    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = id;
    touchDown.tapCount = 1;
    touchDown.physPoint = GetPhysicalPoint(point);
    touchDown.point = GetVirtualPoint(touchDown.physPoint);

    ProcessInput(touchDown);
}

void TouchAction::TouchDown(const Vector<String> &controlPath)
{
    TouchDown(FindControlPosition(controlPath));
}

void TouchAction::TouchUp()
{
    UIEvent touchUp;
    if(!AutotestingSystem::Instance()->FindTouch(id, touchUp))
    {
        Logger::Error("TouchAction::TouchUp touch down not found");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = id;

    ProcessInput(touchUp);
}

void TouchAction::TouchMove(const Vector2 &point)
{
    //Logger::Debug("TouchAction::TouchMove point=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    touchMove.tid = id;
    touchMove.tapCount = 1;
    touchMove.physPoint = GetPhysicalPoint(point);
    touchMove.point = GetVirtualPoint(touchMove.physPoint);
        
    if(AutotestingSystem::Instance()->IsTouchDown(id))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
        ProcessInput(touchMove);
    }
    else
    {
#ifdef __DAVAENGINE_IPHONE__        
        Logger::Warning("TouchAction::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
        touchMove.phase = UIEvent::PHASE_MOVE;
        ProcessInput(touchMove);
#endif
    }
}

Vector2 TouchAction::GetPhysicalPoint(const Vector2 &virtualPoint)
{
    Vector2 physicalPoint;
    
    float32 inputWidth;
    float32 inputHeight;
    if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT || Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN)
	{
		inputWidth = Core::Instance()->GetPhysicalScreenWidth();
		inputHeight = Core::Instance()->GetPhysicalScreenHeight();
	}
	else
	{
		inputWidth = Core::Instance()->GetPhysicalScreenHeight();
		inputHeight = Core::Instance()->GetPhysicalScreenWidth();
	}
    
    Vector2 inputOffset;
    float32 w, h;
	w = (float32)Core::Instance()->GetVirtualScreenWidth() / (float32)inputWidth;
	h = (float32)Core::Instance()->GetVirtualScreenHeight() / (float32)inputHeight;
	inputOffset.x = inputOffset.y = 0;
	if(w > h)
	{
		inputOffset.y = 0.5f * ((float32)Core::Instance()->GetVirtualScreenHeight() - (float32)inputHeight * w);
	}
	else
	{
		inputOffset.x = 0.5f * ((float32)Core::Instance()->GetVirtualScreenWidth() - (float32)inputWidth * h);
	}
    
    float32 virtualToPhysicalFactor = Core::Instance()->GetVirtualToPhysicalFactor();
    
    if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT)
	{
        physicalPoint.x = (virtualPoint.y - inputOffset.y)*virtualToPhysicalFactor;
        physicalPoint.y = inputWidth - (virtualPoint.x - inputOffset.x)*virtualToPhysicalFactor;
        
//		virtualPoint.x = (inputWidth - physicalPoint.y);
//		virtualPoint.y = (physicalPoint.x);
	}
	else if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT)
	{
        physicalPoint.x = inputHeight - (virtualPoint.y - inputOffset.y)*virtualToPhysicalFactor;
        physicalPoint.y = (virtualPoint.x - inputOffset.x)*virtualToPhysicalFactor;
        
//		virtualPoint.x = (physicalPoint.y);
//		virtualPoint.y = (inputHeight - physicalPoint.x);
	}
	else
	{
        physicalPoint = (virtualPoint - inputOffset) * virtualToPhysicalFactor;
//		virtualPoint = physicalPoint;
	}
	
//	virtualPoint *= scaleFactor;
//	virtualPoint += inputOffset;
    
    return physicalPoint;
}
    
Vector2 TouchAction::GetVirtualPoint(const Vector2 &physicalPoint)
{
    Vector2 virtualPoint;
    float32 inputWidth;
    float32 inputHeight;
    if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT || Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN)
	{
		inputWidth = Core::Instance()->GetPhysicalScreenWidth();
		inputHeight = Core::Instance()->GetPhysicalScreenHeight();
	}
	else
	{
		inputWidth = Core::Instance()->GetPhysicalScreenHeight();
		inputHeight = Core::Instance()->GetPhysicalScreenWidth();
	}
    Vector2 inputOffset;
    float32 w, h;
	w = (float32)Core::Instance()->GetVirtualScreenWidth() / (float32)inputWidth;
	h = (float32)Core::Instance()->GetVirtualScreenHeight() / (float32)inputHeight;
	inputOffset.x = inputOffset.y = 0;
	if(w > h)
	{
		inputOffset.y = 0.5f * ((float32)Core::Instance()->GetVirtualScreenHeight() - (float32)inputHeight * w);
	}
	else
	{
		inputOffset.x = 0.5f * ((float32)Core::Instance()->GetVirtualScreenWidth() - (float32)inputWidth * h);
	}
    float32 physicalToVirtualFactor = Core::Instance()->GetPhysicalToVirtualFactor();
    
    if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT)
	{
		virtualPoint.x = (inputWidth - physicalPoint.y);
		virtualPoint.y = (physicalPoint.x);
	}
	else if(Core::Instance()->GetScreenOrientation() == Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT)
	{
		virtualPoint.x = (physicalPoint.y);
		virtualPoint.y = (inputHeight - physicalPoint.x);
	}
	else
	{
		virtualPoint = physicalPoint;
	}
	
	virtualPoint *= physicalToVirtualFactor;
	virtualPoint += inputOffset;
    return virtualPoint;
}

String TouchAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s id=%d", baseStr.c_str(), id);
}

//----------------------------------------------------------------------

TouchDownAction::TouchDownAction(const Vector2 &_point, int32 _id) : TouchAction(_id)
    , point(_point)
{
}

TouchDownAction::~TouchDownAction()
{
}

void TouchDownAction::Execute()
{
    //Logger::Debug("TouchDownAction::Execute point=(%f, %f)",point.x, point.y);
    TouchDown(point);
    Action::Execute();
}

String TouchDownAction::Dump()
{
	String baseStr = TouchAction::Dump();
	return Format("%s point=(%.2f, %.2f)", baseStr.c_str(), point.x, point.y);
}

//----------------------------------------------------------------------
TouchDownControlAction::TouchDownControlAction(const String &_controlName, int32 _id) : TouchAction(_id)
{
    controlPath.push_back(_controlName);
}

TouchDownControlAction::TouchDownControlAction(const Vector<String> &_controlPath, int32 _id) : TouchAction(_id)
    , controlPath(_controlPath)
{
}

TouchDownControlAction::~TouchDownControlAction()
{
}

void TouchDownControlAction::Execute()
{
    TouchDown(controlPath);
    Action::Execute();
}

String TouchDownControlAction::Dump()
{
	String baseStr = TouchAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//----------------------------------------------------------------------
TouchUpAction::TouchUpAction(int32 _id) : TouchAction(_id)
{
}
TouchUpAction::~TouchUpAction()
{
}

void TouchUpAction::Execute()
{
    //Logger::Debug("TouchUpAction::Execute");
    TouchUp();
    Action::Execute();
}
//----------------------------------------------------------------------

TouchMoveAction::TouchMoveAction(const Vector2 &_point, float32 _moveTime, int32 _id) : TouchAction(_id)
    , point(_point)
    , moveTime(_moveTime)
{
}
TouchMoveAction::~TouchMoveAction()
{
}

void TouchMoveAction::Execute()
{
    Action::Execute();
}

String TouchMoveAction::Dump()
{
	String baseStr = TouchAction::Dump();
	return Format("%s point=(%.2f, %.2f) time=%.2f", baseStr.c_str(), point.x, point.y, moveTime);
}

void TouchMoveAction::Update(float32 timeElapsed)
{
    if(!isExecuted)
    {
        moveTime -= timeElapsed;

        if(!TestCondition())
        {
            Vector2 newPoint;
            UIEvent prevTouch;
            if(AutotestingSystem::Instance()->FindTouch(id, prevTouch))
            {
                //TODO: point or physPoint?
                newPoint = prevTouch.point + ((point - prevTouch.point)*timeElapsed/moveTime);
            }
            else
            {
                // mouse move
                Vector2 prevPoint = AutotestingSystem::Instance()->GetMousePosition();
                newPoint = prevPoint +  ((point - prevPoint)*timeElapsed/moveTime);
            }
            TouchMove(newPoint);
        }
        Action::Update(timeElapsed);
    }
}

bool TouchMoveAction::TestCondition()
{
    return (moveTime <= 0.0f);
}

//----------------------------------------------------------------------

TouchMoveControlAction::TouchMoveControlAction(const String &_controlName, float32 _moveTime, int32 _id) : TouchMoveAction(Vector2(), _moveTime, _id)
{
    controlPath.push_back(_controlName);
}

TouchMoveControlAction::TouchMoveControlAction(const Vector<String> &_controlPath, float32 _moveTime, int32 _id) : TouchMoveAction(Vector2(), _moveTime, _id)
    , controlPath(_controlPath)
{

}

TouchMoveControlAction::~TouchMoveControlAction()
{
}

void TouchMoveControlAction::Execute()
{
    point = FindControlPosition(controlPath);
    TouchMoveAction::Execute();
}

String TouchMoveControlAction::Dump()
{
	String baseStr = TouchMoveAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//----------------------------------------------------------------------
ScrollControlAction::ScrollControlAction(const String &_controlName, int32 _id, float32 timeout) : WaitAction(timeout)
    , id(_id)
    , currentAction(NULL)
{
    controlPath.push_back(_controlName);
}
ScrollControlAction::ScrollControlAction(const Vector<String> &_controlPath, int32 _id, float32 timeout) : WaitAction(timeout)
    , controlPath(_controlPath)
    , id(_id)
    , currentAction(NULL)
    , isFound(false)
{
}

ScrollControlAction::~ScrollControlAction()
{
    for(Deque<Action*>::iterator it = actions.begin(); it != actions.end(); ++it)
    {
        SafeRelease(*it);
    }
    actions.clear();
}

void ScrollControlAction::Update(float32 timeElapsed)
{
    WaitAction::Update(timeElapsed);
    if(!isExecuted)
    {
        if(actions.empty())
        {
            isFound = (FindControl(controlPath) != NULL);
            if(!isFound)
            {
                //add: touch down, touch move, touch up
                actions.push_back(new TouchDownAction(touchDownPoint, id));
                actions.push_back(new TouchMoveAction(touchUpPoint, 0.2f, id));
                actions.push_back(new TouchUpAction(id));
            }
        }
        else
        {
            // update touch move
            if(!currentAction)
            {
                currentAction = actions.front();
                if(currentAction)
                {
                    currentAction->Execute();
                }
            }

            if(currentAction)
            {
                if(currentAction->IsExecuted())
                {
                    SafeRelease(currentAction);
                    actions.pop_front();
                }
                else
                {
                    currentAction->Update(timeElapsed);
                }
            }
            else
            {
                actions.pop_front();
            }
        }
    }
}

void ScrollControlAction::Execute()
{
    FindScrollPoints();
    WaitAction::Execute();
}

String ScrollControlAction::Dump()
{
	String baseStr = WaitAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

bool ScrollControlAction::TestCondition()
{
    if(WaitAction::TestCondition())
    {
        AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s timeout", PathToString(controlPath).c_str()));
        return true;
    }
    return isFound;
    //return (FindControl(controlPath) != NULL);
}

void ScrollControlAction::FindScrollPoints()
{
    if(controlPath.empty()) return;

    Vector<String> parentPath;
    for(int32 i = 0; i < controlPath.size() - 1; ++i)
    {
        parentPath.push_back(controlPath[i]);
    }
    String indexStr = controlPath.back();
    int32 index = atoi(indexStr.c_str());

    UIControl* parent = NULL;
    if(parentPath.empty())
    {
        // parent is screen
        AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s unable to scroll screen", PathToString(controlPath).c_str()));
    }
    else
    {
        parent = FindControl(parentPath);
        // parent is something inside screen
        UIList* parentList = dynamic_cast<UIList*>(parent);
        if(parentList)
        {
            // parent is list
            Rect parentRect(parentList->GetGeometricData().GetUnrotatedRect());

            //TODO: find min and max visible index
            int32 minVisibleIndex = 10000;
            int32 maxVisibleIndex = 0;
            const List<UIControl*> &cells = parentList->GetVisibleCells();
            for(List<UIControl*>::const_iterator it = cells.begin(); it != cells.end(); ++it)
            {
                UIListCell* cell = dynamic_cast<UIListCell*>(*it);
                if(cell && IsInside(parentList, cell))
                {
                    int32 cellIndex = cell->GetIndex();
                    if(cellIndex < minVisibleIndex)
                    {
                        minVisibleIndex = cellIndex;
                    }
                    
                    if(maxVisibleIndex < cellIndex)
                    {
                        maxVisibleIndex = cellIndex;
                    }
                }
            }

            if(parentList->GetOrientation() == UIList::ORIENTATION_HORIZONTAL)
            {
                // horizontal
                if(maxVisibleIndex < index)
                {
                    // touch up < down 
                    touchDownPoint = Vector2(parentRect.x + 0.55f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
                    touchUpPoint = Vector2(parentRect.x + 0.45f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
                }
                else
                {
                    // touch down < up
                    touchDownPoint = Vector2(parentRect.x + 0.45f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
                    touchUpPoint = Vector2(parentRect.x + 0.55f*parentRect.dx, parentRect.y + 0.5f*parentRect.dy);
                }
            }
            else
            {
                // vertical
                if(maxVisibleIndex < index)
                {
                    // touch up < down 
                    touchDownPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.55f*parentRect.dy);
                    touchUpPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.45f*parentRect.dy);
                }
                else
                {
                    // touch down < up
                    touchDownPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.45f*parentRect.dy);
                    touchUpPoint = Vector2(parentRect.x + 0.5f*parentRect.dx, parentRect.y + 0.55f*parentRect.dy);
                }
                
            }
            Logger::Debug("ScrollControlAction::FindScrollPoints %s scroll from (%f, %f) to (%f, %f)", PathToString(controlPath).c_str(), touchDownPoint.x, touchDownPoint.y, touchUpPoint.x, touchUpPoint.y);
        }
        else
        {
            // parent is not list
            //TODO: scroll controls
            AutotestingSystem::Instance()->OnError(Format("ScrollControlAction %s unable to scroll control", PathToString(controlPath).c_str()));
        }
    }
}

};

#endif //__DAVAENGINE_AUTOTESTING__