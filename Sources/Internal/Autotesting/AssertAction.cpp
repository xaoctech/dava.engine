#include "Autotesting/AssertAction.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"

namespace DAVA
{

const VariantType &ControlGetter::Get()
{
    if(!isExecuted)
    {
        Execute();
    }
    return Getter::Get();
}

//-----------------------------------------------------------------

void ControlTextGetter::Execute()
{
    value.SetWideString(GetText(controlPath));
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

void ControlBoolGetter::Execute()
{
    value.SetBool(FindControl(controlPath) != NULL);
    ControlGetter::Execute();
}

//-----------------------------------------------------------------

VTAssertAction::VTAssertAction() : Action()
    , expected(NULL)
    , actual(NULL)
{
}

VTAssertAction::~VTAssertAction()
{
    SafeRelease(expected); 
    SafeRelease(actual);
}

void VTAssertAction::SetExpectedGetter(Getter *_expected)
{
    SafeRelease(expected);
    expected = SafeRetain(_expected);
}
void VTAssertAction::SetActualGetter(Getter *_actual)
{
    SafeRelease(actual);
    actual = SafeRetain(_actual);
}

void VTAssertAction::Execute()
{
    bool isPassed = false;
    if(actual && expected)
    {
        isPassed = (actual->Get() == expected->Get());
    }
    AutotestingSystem::Instance()->OnTestAssert("AssertAction::Execute", isPassed);
    Action::Execute();
}

//-----------------------------------------------------------------

// template <class T> const T& ControlGetter<T>::Get()
// {
//     if(!isExecuted)
//     {
//         Execute();
//     }
//     return Getter::Get();
// }

//-----------------------------------------------------------------

// void ControlTextGetter::Execute()
// {
//     value = GetText(controlPath);
//     ControlGetter::Execute();
// }

//-----------------------------------------------------------------

// void ControlBoolGetter::Execute()
// {
//     value = (FindControl(controlPath) != NULL);
//     ControlGetter::Execute();
// }

//-----------------------------------------------------------------

// template <class T> AssertAction<T>::AssertAction() : Action()
//     , expected(NULL)
//     , actual(NULL)
// {
// }
// 
// template <class T> AssertAction<T>::~AssertAction()
// {
//     SafeRelease(expected); 
//     SafeRelease(actual);
// }
// 
// template <class T> void AssertAction<T>::Execute()
// {
//     bool isPassed = false;
//     if(actual && expected)
//     {
//         isPassed = (actual->Get() == expected->Get());
//     }
//     AutotestingSystem::Instance()->OnTestAssert("AssertAction::Execute", isPassed);
//     Action::Execute();
// }

//-----------------------------------------------------------------------------------
// AssertTextAction::AssertTextAction(const WideString &_expected, const Vector<String> &_actualControlPath)
// {
//     // compare value with result of getter
//     expected = new Getter<WideString>(_expected);
//     actual = new ControlTextGetter(_actualControlPath);
// }
// 
// AssertTextAction::AssertTextAction(const Vector<String> &_expectedControlPath, const Vector<String> &_actualControlPath)
// {
//     // compare results of getters
//     expected = new ControlTextGetter(_expectedControlPath);
//     actual = new ControlTextGetter(_actualControlPath);
// }
// 
// AssertTextAction::~AssertTextAction()
// {
// }

//-----------------------------------------------------------------------------------
// AssertBoolAction::AssertBoolAction(bool _expected, const Vector<String> &_actualControlPath)
// {
//     // compare value with result of getter
//     expected = new Getter<bool>(_expected);
//     actual = new ControlBoolGetter(_actualControlPath);
// }
// 
// AssertBoolAction::AssertBoolAction(const Vector<String> &_expectedControlPath, const Vector<String> &_actualControlPath)
// {
//     // compare results of getters
//     expected = new ControlBoolGetter(_expectedControlPath);
//     actual = new ControlBoolGetter(_actualControlPath);
// }
// 
// AssertBoolAction::~AssertBoolAction()
// {
// }

};

#endif //__DAVAENGINE_AUTOTESTING__