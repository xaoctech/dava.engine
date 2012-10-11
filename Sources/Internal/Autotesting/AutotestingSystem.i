/* File : AutotestingSystem.i */
%module AutotestingSystem
%import Base/Singleton.h
%import Base/BaseTypes.h

%{
#include "AutotestingSystemLua.h"
%}

%template(Singleton_Autotesting) DAVA::Singleton<DAVA::AutotestingSystemLua>;

/* Let's just grab the original header file here */
%include "AutotestingSystemLua.h"