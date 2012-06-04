#include "Autotesting/TouchTest.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{

TouchTest::TouchTest(int32 _id) : Test()
    , id(_id)
{
}

TouchTest::~TouchTest()
{
}
    
void TouchTest::TouchDown(const Vector2 &point)
{
    Logger::Debug("TouchTest::TouchDown point=(%f, %f) id=%d", point.x, point.y, id);

    //TODO: multitouch

    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = id;
    touchDown.tapCount = 1;
    touchDown.physPoint = GetPhysPoint(point);
    touchDown.point = point;

    ProcessInput(touchDown);
}

void TouchTest::TouchDown(const String &controlName)
{
    Logger::Debug("TouchTest::TouchDown controlName=%s",controlName.c_str());
    TouchDown(FindControlPositionByName(controlName));
}

void TouchTest::TouchUp()
{
    Logger::Debug("TouchTest::TouchUp");

//     if(!isDrag)
//     {
//         Logger::Warning("TouchTest::TouchUp multitouch not supported");
//     }

    UIEvent touchUp;
    if(AutotestingSystem::Instance()->FindTouch(id, touchUp))
    {

    }
    else
    {
        Logger::Error("TouchTest::TouchUp");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = id;

    ProcessInput(touchUp);
}

void TouchTest::TouchMove(const Vector2 &point)
{
    //TODO: PHASE_DRAG PHASE_MOVE
    Logger::Debug("TouchTest::TouchMove toPoint=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
//     if(isDrag)
//     {
        touchMove.phase = UIEvent::PHASE_DRAG;
//     }
//     else
//     {
//         touchMove.phase = UIEvent::PHASE_MOVE;
//     }
    touchMove.tid = id;
    touchMove.tapCount = 1;
    touchMove.physPoint = GetPhysPoint(point);
    touchMove.point = point;

    ProcessInput(touchMove);
}

Vector2 TouchTest::GetPhysPoint(const Vector2 &p)
{
    return p; //TODO: calculate physical coords from virtual
}

//----------------------------------------------------------------------

TouchDownTest::TouchDownTest(const Vector2 &_point, int32 _id) : TouchTest(_id)
    , point(_point)
{
}

TouchDownTest::~TouchDownTest()
{
}

void TouchDownTest::Execute()
{
    Logger::Debug("TouchDownTest::Execute point=(%f, %f)",point.x, point.y);
    TouchDown(point);
    Test::Execute();
}

//----------------------------------------------------------------------
TouchControlTest::TouchControlTest(const String &_controlName, int32 _id) : TouchTest(_id)
    , controlName(_controlName)
{
}

TouchControlTest::~TouchControlTest()
{
}

void TouchControlTest::Execute()
{
    Logger::Debug("TouchControlTest::Execute controlName=%s", controlName.c_str());
    TouchDown(controlName);
    Test::Execute();
}

//----------------------------------------------------------------------
TouchUpTest::TouchUpTest(int32 _id) : TouchTest(_id)
{
}
TouchUpTest::~TouchUpTest()
{
}

void TouchUpTest::Execute()
{
    Logger::Debug("TouchUpTest::Execute");
    TouchUp();
    Test::Execute();
}
//----------------------------------------------------------------------

TouchMoveTest::TouchMoveTest(const Vector2 &_point, float32 _moveTime, int32 _id) : TouchTest(_id)
    , point(_point)
    , moveTime(_moveTime)
{
}
TouchMoveTest::~TouchMoveTest()
{
}

void TouchMoveTest::Execute()
{
    Logger::Debug("TouchMoveTest::Execute");
    Test::Execute();
}

void TouchMoveTest::Update(float32 timeElapsed)
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
                TouchMove(newPoint);
            }
        }
        Test::Update(timeElapsed);
    }
}

bool TouchMoveTest::TestCondition()
{
    return (moveTime <= 0.0f);
}

};

#endif //__DAVAENGINE_AUTOTESTING__