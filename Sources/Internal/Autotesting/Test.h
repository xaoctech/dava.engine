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
#ifndef __DAVAENGINE_TEST_H__
#define __DAVAENGINE_TEST_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA
{

class Test : public BaseObject
{
public:
    Test();
    virtual ~Test();

    virtual void Update(float32 timeElapsed);
    virtual void Execute();
    inline bool IsExecuted() { return isExecuted; };

protected:
    virtual bool TestCondition();

    void SetText(const String &controlName, const WideString &text);
    WideString GetText(const String &controlName);

    void ProcessInput(const UIEvent &input);
    Vector2 FindControlPositionByName(const String& controlName);
    UIControl* FindByName(const String& controlName);

    bool isExecuted;
};

class KeyPressTest : public Test
{
public:
    KeyPressTest(char16 _keyChar);
    virtual ~KeyPressTest();

    virtual void Execute();
protected:
    void KeyPress(char16 keyChar);
    char16 keyChar;
};

class WaitTest : public Test
{
public:
    WaitTest(float32 _waitTime);
    virtual ~WaitTest();

    virtual void Execute();
    virtual void Update(float32 timeElapsed);
protected:
    virtual bool TestCondition();
    float32 waitTime;
};

class WaitForUITest : public Test
{
public:
    WaitForUITest(const String& _controlName);
    virtual ~WaitForUITest();

    virtual void Execute();
protected:
    virtual bool TestCondition();
    String controlName;
};

class SetTextTest : public Test
{
public:
    SetTextTest(const String& _controlName, const WideString &_text);
    virtual ~SetTextTest();

    virtual void Execute();
protected:
    String controlName;
    WideString text;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_TEST_H__