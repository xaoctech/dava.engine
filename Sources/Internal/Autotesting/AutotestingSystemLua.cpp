#include "Autotesting/AutotestingSystemLua.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "AutotestingSystem.h"

#include "Utils/Utils.h"

//TODO: move all wrappers to separate class?
#include "Action.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

// directly include wrapped module here to compile only if __DAVAENGINE_AUTOTESTING__ is defined
#include "AutotestingSystem_wrap.cxx"
//extern "C" int luaopen_AutotestingSystem(lua_State *l); // declare the wrapped module

namespace DAVA
{
    
AutotestingSystemLua::AutotestingSystemLua() : luaState(NULL), delegate(NULL)
{
}

AutotestingSystemLua::~AutotestingSystemLua()
{
    if(luaState)
    {
        lua_close(luaState);
        luaState = NULL;
    }
}

void AutotestingSystemLua::SetDelegate(AutotestingSystemLuaDelegate *_delegate)
{
    delegate = _delegate;
}
    
void AutotestingSystemLua::InitFromFile(const String &luaFilePath)
{
    if(!luaState)
    {
        luaState = lua_open();
        luaL_openlibs(luaState);
        LoadWrappedLuaObjects();
        RunScriptFromFile("~res:/Autotesting/Scripts/autotesting_api.lua");
        
        LoadScriptFromFile(luaFilePath);
        
        //TODO: get test name (the name of function)
        String path;
        String filename;
        FileSystem::Instance()->SplitPath(luaFilePath, path, filename);
        AutotestingSystem::Instance()->Init(filename);
        
        AutotestingSystem::Instance()->RunTests();
    }
}

void AutotestingSystemLua::StartTest()
{
    Logger::Debug("AutotestingSystemLua::StartTest");
    RunScript();
}

void AutotestingSystemLua::Update(float32 timeElapsed)
{
    RunScript("ResumeTest()"); //TODO: time 
}
    
float32 AutotestingSystemLua::GetTimeElapsed()
{
    return SystemTimer::FrameDelta();
}
    
void AutotestingSystemLua::StopTest()
{
    Logger::Debug("AutotestingSystemLua::StopTest");
    AutotestingSystem::Instance()->OnTestsFinished();
}
    
bool AutotestingSystemLua::FindControl(const String &path)
{
    //TODO: parse path
    Vector<String> controlPath;
    controlPath.push_back(path);
    return (Action::FindControl(controlPath) != NULL);
}
    
void AutotestingSystemLua::LoadWrappedLuaObjects()
{
    if(!luaState) return; //TODO: report error?
    
    luaopen_AutotestingSystem(luaState);	// load the wrappered module
    if(delegate)
    {
        delegate->LoadWrappedLuaObjects();
    }
}

bool AutotestingSystemLua::LoadScript(const String &luaScript)
{
    if(!luaState) return false;
    
    if(luaL_loadstring(luaState, luaScript.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScript Error: unable to load %s", luaScript.c_str());
        return false;
    }
    return true;
}
    
bool AutotestingSystemLua::LoadScriptFromFile(const String &luaFilePath)
{
    if(!luaState) return false;
    
    String realPath = FileSystem::Instance()->SystemPathForFrameworkPath(luaFilePath);
    if(luaL_loadfile(luaState, realPath.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScriptFromFile Error: unable to load %s", realPath.c_str());
        return false;
    }
    return true;
}
    
void AutotestingSystemLua::RunScriptFromFile(const String &luaFilePath)
{
    Logger::Debug("AutotestingSystemLua::RunScriptFromFile %s", luaFilePath.c_str());
    if(LoadScriptFromFile(luaFilePath))
    {
        RunScript();
    }
}
    
void AutotestingSystemLua::RunScript(const DAVA::String &luaScript)
{
    Logger::Debug("AutotestingSystemLua::RunScript %s", luaScript.c_str());
    if(LoadScript(luaScript))
    {
        RunScript();
    }
}
    
void AutotestingSystemLua::RunScript()
{
    Logger::Debug("AutotestingSystemLua::RunScript");
    lua_pcall(luaState, 0, 0, 0); //TODO: LUA_MULTRET?
}

};

#endif //__DAVAENGINE_AUTOTESTING__