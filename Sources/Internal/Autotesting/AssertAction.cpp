#include "Autotesting/AssertAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{
    
Getter::Getter()
{
    SetName("Getter");
}

Getter::Getter(const VariantType &_value) : Action(), value(_value)
{
    SetName("Getter");
}

String Getter::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s type=%d", baseStr.c_str(), value.type);
}

//-----------------------------------------------------------------

ControlGetter::ControlGetter(const Vector<String> &_controlPath) : Getter(), controlPath(_controlPath)
{
    SetName("ControlGetter");
}
    
const VariantType &ControlGetter::Get()
{
    if(!isExecuted)
    {
        Execute();
    }
    return Getter::Get();
}

String ControlGetter::Dump()
{
	String baseStr = Getter::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//-----------------------------------------------------------------
ControlTextGetter::ControlTextGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath)
{
    SetName("ControlTextGetter");
}
    
void ControlTextGetter::Execute()
{
    value.SetWideString(GetText(controlPath));
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

ControlBoolGetter::ControlBoolGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath)
{
    SetName("ControlBoolGetter");
};
    
void ControlBoolGetter::Execute()
{
    value.SetBool(FindControl(controlPath) != NULL);
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

AssertAction::AssertAction(const String &_message) : Action()
    , expected(NULL)
    , actual(NULL)
    , message(_message)
{
    SetName("AssertAction");    
}

AssertAction::~AssertAction()
{
    SafeRelease(expected); 
    SafeRelease(actual);
}

void AssertAction::SetExpectedGetter(Getter *_expected)
{
    SafeRelease(expected);
    expected = SafeRetain(_expected);
}
void AssertAction::SetActualGetter(Getter *_actual)
{
    SafeRelease(actual);
    actual = SafeRetain(_actual);
}

void AssertAction::Execute()
{
    bool isPassed = false;
    if(actual && expected)
    {
        isPassed = (actual->Get() == expected->Get());
    }
	Action::Execute();
    AutotestingSystem::Instance()->OnTestStep(message, isPassed);
}

String AssertAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s message=%s", baseStr.c_str(), message.c_str());
}

};

#endif //__DAVAENGINE_AUTOTESTING__