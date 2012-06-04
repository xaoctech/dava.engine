/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Dmitry Shpakov 
=====================================================================================*/

#ifndef __DAVAENGINE_TOUCH_TEST_H__
#define __DAVAENGINE_TOUCH_TEST_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/Test.h"

namespace DAVA
{

class TouchTest : public Test
{
public:
    TouchTest(int32 _id);
    virtual ~TouchTest();

protected:
    void TouchDown(const Vector2 &point);
    void TouchDown(const String &controlName);
    void TouchUp();
    void TouchMove(const Vector2 &point);

    Vector2 GetPhysPoint(const Vector2 &p);

    int32 id;
};

class TouchDownTest : public TouchTest
{
public:
    TouchDownTest(const Vector2 &_point, int32 _id);
    virtual ~TouchDownTest();

    virtual void Execute();
protected:
    Vector2 point;
};

class TouchControlTest : public TouchTest
{
public:
    TouchControlTest(const String &_controlName, int32 _id);
    virtual ~TouchControlTest();

    virtual void Execute();
protected:
    String controlName;
};

class TouchUpTest : public TouchTest
{
public:
    TouchUpTest(int32 id);
    virtual ~TouchUpTest();

    virtual void Execute();
};

class TouchMoveTest : public TouchTest
{
public:
    TouchMoveTest(const Vector2 &_point, float32 _moveTime, int32 _id);
    virtual ~TouchMoveTest();

    virtual void Execute();
    virtual void Update(float32 timeElapsed);

protected:
    virtual bool TestCondition();
    Vector2 point;
    float32 moveTime;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_TOUCH_TEST_H__