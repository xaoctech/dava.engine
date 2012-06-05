#include "Autotesting/TouchAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

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
    touchDown.physPoint = GetPhysPoint(point);
    touchDown.point = point;

    ProcessInput(touchDown);
}

void TouchAction::TouchDown(const Vector<String> &controlPath)
{
    TouchDown(FindControlPosition(controlPath));
}

void TouchAction::TouchUp()
{
    Logger::Debug("TouchAction::TouchUp");

    UIEvent touchUp;
    if(AutotestingSystem::Instance()->FindTouch(id, touchUp))
    {

    }
    else
    {
        Logger::Error("TouchAction::TouchUp");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = id;

    ProcessInput(touchUp);
}

void TouchAction::TouchMove(const Vector2 &point)
{
    //TODO: PHASE_DRAG PHASE_MOVE
    Logger::Debug("TouchAction::TouchMove toPoint=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    if(AutotestingSystem::Instance()->IsTouchDown(id))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
    }
    else
    {
         touchMove.phase = UIEvent::PHASE_MOVE;
    }
    touchMove.tid = id;
    touchMove.tapCount = 1;
    touchMove.physPoint = GetPhysPoint(point);
    touchMove.point = point;

    ProcessInput(touchMove);
}

Vector2 TouchAction::GetPhysPoint(const Vector2 &p)
{
    return p; //TODO: calculate physical coords from virtual
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
    Logger::Debug("TouchDownAction::Execute point=(%f, %f)",point.x, point.y);
    TouchDown(point);
    Action::Execute();
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

//----------------------------------------------------------------------
TouchUpAction::TouchUpAction(int32 _id) : TouchAction(_id)
{
}
TouchUpAction::~TouchUpAction()
{
}

void TouchUpAction::Execute()
{
    Logger::Debug("TouchUpAction::Execute");
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

};

#endif //__DAVAENGINE_AUTOTESTING__