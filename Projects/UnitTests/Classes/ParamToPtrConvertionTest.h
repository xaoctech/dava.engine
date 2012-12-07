#ifndef __PARAMTOPTRCONVERTIONTEST_H__
#define __PARAMTOPTRCONVERTIONTEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class ParamToPtrConvertionTest : public TestTemplate<ParamToPtrConvertionTest>
{

public:
    ParamToPtrConvertionTest();

    void CheckConvertation( PerfFuncData * data );
};

#endif // __PARAMTOPTRCONVERTIONTEST_H__
