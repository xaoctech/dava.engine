#include "Autotesting/AutotestingSystemLua.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "AutotestingSystem.h"

#include "Utils/Utils.h"

//TODO: move all wrappers to separate class?
#include "Action.h"
#include "TouchAction.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

// directly include wrapped module here to compile only if __DAVAENGINE_AUTOTESTING__ is defined
//#include "AutotestingSystem_wrap.cxx"

//#include "VectorSwig.cpp"
//#include "UIControlSwig.cpp"
//#include "AutotestingSystemSwig.cpp"

extern "C" int luaopen_AutotestingSystem(lua_State *l);
extern "C" int luaopen_UIControl(lua_State *l); 
extern "C" int luaopen_Rect(lua_State *l);
extern "C" int luaopen_Vector(lua_State *l);

namespace DAVA
{
        
AutotestingSystemLua::AutotestingSystemLua() : luaState(NULL), delegate(NULL), autotestingLocalizationSystem(NULL)
{
    autotestingLocalizationSystem = new LocalizationSystem();
}

AutotestingSystemLua::~AutotestingSystemLua()
{
    SafeRelease(autotestingLocalizationSystem);
    
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
    Logger::Debug("AutotestingSystemLua::InitFromFile luaFilePath=%s", luaFilePath.c_str());
    if(!luaState)
    {
        autotestingLocalizationSystem->SetCurrentLocale(LocalizationSystem::Instance()->GetCurrentLocale());
        autotestingLocalizationSystem->InitWithDirectory("~res:/Autotesting/Strings");
        
        bool isOk = true;
        luaState = lua_open();
        luaL_openlibs(luaState);
        String errors;
        
        if(isOk)
        {
            isOk = LoadWrappedLuaObjects();
        }
        else
        {
            errors += " luaL_openlibs failed";
        }
        
        if(isOk)
        {
            isOk = RunScriptFromFile("~res:/Autotesting/Scripts/autotesting_api.lua");
        }
        else
        {
            errors += ", LoadWrappedLuaObjects failed";
        }
        
        String pathToAutotesting = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/Autotesting/");
        String setPackagePathScript = Format("SetPackagePath(\"%s\")", pathToAutotesting.c_str());
        if(isOk)
        {
            isOk = RunScript(setPackagePathScript);
        }
        else
        {
            errors += ", autotesting_api.lua failed";
        }
        
        if(isOk)
        {
            isOk = LoadScriptFromFile(luaFilePath);
        }
        else
        {
            errors += ", " + setPackagePathScript + " failed";
        }
        
        if(isOk)
        {
            //TODO: get test name (the name of function)
            String path;
            String filename;
            FileSystem::Instance()->SplitPath(luaFilePath, path, filename);
            AutotestingSystem::Instance()->Init(filename);
        }
        else
        {
            errors += ", " + luaFilePath + " failed";
        }
        
        if(isOk)
        {
            AutotestingSystem::Instance()->RunTests();
        }
        else
        {
            AutotestingSystem::Instance()->OnError(errors);
        }
    }
}

void AutotestingSystemLua::StartTest()
{
    Logger::Debug("AutotestingSystemLua::StartTest");
    RunScript();
}
    
void AutotestingSystemLua::WaitForMaster()
{
    Logger::Debug("AutotestingSystemLua::WaitForMaster");
    AutotestingSystem::Instance()->InitMultiplayer(false);
    AutotestingSystem::Instance()->RegisterHelperInDB();
}
    
void AutotestingSystemLua::WaitForHelpers(DAVA::int32 helpersCount)
{
    Logger::Debug("AutotestingSystemLua::WaitForHelpers %d", helpersCount);
    AutotestingSystem::Instance()->InitMultiplayer(true);
    AutotestingSystem::Instance()->RegisterMasterInDB(helpersCount);
}

void AutotestingSystemLua::Update(float32 timeElapsed)
{
    RunScript("ResumeTest()"); //TODO: time 
}
    
float32 AutotestingSystemLua::GetTimeElapsed()
{
    return SystemTimer::FrameDelta();
}
    
void AutotestingSystemLua::OnError(const String &errorMessage)
{
    Logger::Debug("AutotestingSystemLua::OnError %s", errorMessage.c_str());
    AutotestingSystem::Instance()->OnError(errorMessage);
}
    
void AutotestingSystemLua::OnTestStep(const String &stepName, bool isPassed, const String &error)
{
    Logger::Debug("AutotestingSystemLua::OnTestStep %s %d %s", stepName.c_str(), isPassed, error.c_str());
    AutotestingSystem::Instance()->OnTestStep(stepName, isPassed, error);
}

void AutotestingSystemLua::OnTestStart(const String &testName)
{
	Logger::Debug("AutotestingSystemLua::OnTestStart %s", testName.c_str());
	AutotestingSystem::Instance()->OnTestStart(testName);
}

void AutotestingSystemLua::OnTestFinished()
{
    Logger::Debug("AutotestingSystemLua::OnTestFinished");
    AutotestingSystem::Instance()->OnTestsFinished();
}
    
void AutotestingSystemLua::OnStepStart(const String &stepName)
{
	Logger::Debug("AutotestingSystemLua::OnStepStart %s", stepName.c_str());
	AutotestingSystem::Instance()->OnStepStart(stepName);
}

void AutotestingSystemLua::Log(const String &level, const String &message)
{
	Logger::Debug("AutotestingSystemLua::Log [%s]%s", level.c_str(), message.c_str());
	AutotestingSystem::Instance()->Log(level, message);
}

UIControl *AutotestingSystemLua::FindControl(const String &path)
{
    Logger::Debug("AutotestingSystemLua::FindControl %s", path.c_str());
    
    Vector<String> controlPath;
    ParsePath(path, controlPath);
    
    return Action::FindControl(controlPath);
}
    
bool AutotestingSystemLua::SetText(const String &path, const String &text)
{
    Logger::Debug("AutotestingSystemLua::SetText %s %s", path.c_str(), text.c_str());
    UITextField *tf = dynamic_cast<UITextField*>(FindControl(path));
    if(tf)
    {
        tf->SetText(StringToWString(text));
        return true;
    }
    return false;
}
    
bool AutotestingSystemLua::CheckText(UIControl *control, const String &expectedText)
{
	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		String actualText = WStringToString(uiStaticText->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiStaticText->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		String actualText = WStringToString(uiTextField->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiTextField->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	return false;
}

bool AutotestingSystemLua::CheckMsgText(UIControl *control, const String &key)
{
	WideString expectedText = StringToWString(key);
	//TODO: check key in localized strings for Lua
	expectedText = autotestingLocalizationSystem->GetLocalizedString(expectedText);

	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		WideString actualText = uiStaticText->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiStaticText->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		WideString actualText = uiTextField->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiTextField->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	return false;
}
    
void AutotestingSystemLua::TouchDown(const Vector2 &point, int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchDown point=(%f,%f) touchId=%d", point.x, point.y, touchId);
      
    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = touchId;
    touchDown.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchDown.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchDown.physPoint, touchDown.point);
        
    Action::ProcessInput(touchDown);
}
    
void AutotestingSystemLua::TouchMove(const Vector2 &point, int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchMove point=(%f,%f) touchId=%d", point.x, point.y, touchId);
    
    //Logger::Debug("TouchAction::TouchMove point=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    touchMove.tid = touchId;
    touchMove.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchMove.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchMove.physPoint, touchMove.point);
    
    if(AutotestingSystem::Instance()->IsTouchDown(touchId))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
        Action::ProcessInput(touchMove);
    }
    else
    {
#ifdef __DAVAENGINE_IPHONE__
        Logger::Warning("AutotestingSystemLua::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
        touchMove.phase = UIEvent::PHASE_MOVE;
        Action::ProcessInput(touchMove);
#endif
    }
}
    
void AutotestingSystemLua::TouchUp(int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchUp touchId=%d", touchId);
    UIEvent touchUp;
    if(!AutotestingSystem::Instance()->FindTouch(touchId, touchUp))
    {
        Logger::Error("TouchAction::TouchUp touch down not found");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = touchId;
    
    Action::ProcessInput(touchUp);
}

void AutotestingSystemLua::ParsePath(const String &path, Vector<String> &parsedPath)
{
    Logger::Debug("AutotestingSystemLua::ParsePath path=%s", path.c_str());
    Split(path, "/", parsedPath);
}
    
bool AutotestingSystemLua::LoadWrappedLuaObjects()
{
    if(!luaState) return false; //TODO: report error?
    
    luaopen_AutotestingSystem(luaState);	// load the wrappered module
    luaopen_UIControl(luaState);	// load the wrappered module
    luaopen_Rect(luaState);	// load the wrappered module
    luaopen_Vector(luaState);	// load the wrappered module
    
    if(delegate)
    {
        delegate->LoadWrappedLuaObjects();
    }
    //TODO: check if modules really loaded
    return true;
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
    
bool AutotestingSystemLua::RunScriptFromFile(const String &luaFilePath)
{
    Logger::Debug("AutotestingSystemLua::RunScriptFromFile %s", luaFilePath.c_str());
    if(LoadScriptFromFile(luaFilePath))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript(const DAVA::String &luaScript)
{
    //Logger::Debug("AutotestingSystemLua::RunScript %s", luaScript.c_str());
    if(LoadScript(luaScript))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript()
{
    //Logger::Debug("AutotestingSystemLua::RunScript");
    lua_pcall(luaState, 0, 0, 0); //TODO: LUA_MULTRET?
    //TODO: check if lua_pcall was successfull
    return true;
}

};

#endif //__DAVAENGINE_AUTOTESTING__