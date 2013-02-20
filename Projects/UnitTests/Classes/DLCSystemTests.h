//
//  DLCSystemTests.h
//  WoTSniperMacOS
//
//  Created by Andrey Panasyuk on 4/12/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __DAVAENGINE_DLCSystemTests_h
#define __DAVAENGINE_DLCSystemTests_h

#include "DLC/DLCSystem.h"
#include "TestTemplate.h"

namespace DAVA
{

class DLCSystemTests: public DLCSystemDelegate
{
public:
	DLCSystemTests();

    void StartTests();
    
    void Test1();
    void Test2();
    void Test3();
    void Test4();
    void Test5();
    void Test6();
    void Test7();
    void Test8();

    void NextTest();
    
    // File with contents of all DLCs has getted 
    virtual void InitCompleted(DLCStatusCode withStatus);
    // Single DLC file has downloaded or end with error
    virtual void DLCCompleted(DLCStatusCode withStatus, uint16 index);
    // All DLC files has downloaded
    virtual void AllDLCCompleted();

	bool IsFinished() { return isFinished; };
	bool IsNeedNextTest() { return needNextTest; };
	uint32 GetCurTestNumber() { return state; };

public:
    enum States
    {
        TEST_1 = 0,
        TEST_2,
        TEST_3,
        TEST_4,
        TEST_5,
        TEST_6,
        TEST_7,
        TEST_8
    };
private:
    States state;
    bool isSucsess;
	bool isFinished;
	bool needNextTest;
};

}// End DAVA

class DLCTest: public TestTemplate<DLCTest>
{
public:
	DLCTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void DLCTestFunction(PerfFuncData * data);

private:
	DLCSystemTests* tests;

	bool isStarted;
};


#endif
