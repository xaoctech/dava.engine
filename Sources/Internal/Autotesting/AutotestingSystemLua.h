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
#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"
#include "Base/Singleton.h"
#include "UI/UIControl.h"


struct lua_State;

namespace DAVA
{
    
#ifndef SWIG
class AutotestingSystemLuaDelegate
{
public:
    virtual void LoadWrappedLuaObjects() = 0;
};
#endif

class AutotestingSystemLua : public Singleton<AutotestingSystemLua>
{
public:
    AutotestingSystemLua();
    ~AutotestingSystemLua();
    
#ifndef SWIG
    void SetDelegate(AutotestingSystemLuaDelegate *_delegate);

    void InitFromFile(const String &luaFilePath);
    
    void StartTest();

    void Update(float32 timeElapsed);
#endif
    
    void StopTest();
    float32 GetTimeElapsed();
    UIControl *FindControl(const String &path);
    void TouchDown(const Vector2 &point, int32 touchId);
    void TouchUp(int32 touchId);
    
protected:
#ifndef SWIG
    void ParsePath(const String &path, Vector<String> &parsedPath);
    
    bool LoadScript(const String &luaScript);
    bool LoadScriptFromFile(const String &luaFilePath);
    void RunScript();
    
    void RunScript(const String &luaScript);
    void RunScriptFromFile(const String &luaFilePath);
    void LoadWrappedLuaObjects();
    
    AutotestingSystemLuaDelegate *delegate;
    lua_State *luaState; //TODO: multiple lua states
#endif
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_LUA_H__