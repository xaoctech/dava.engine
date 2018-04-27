#include "UnitTests/UnitTests.h"

#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

#include <VisualScript/Nodes/VisualScriptBranchNode.h>
#include <VisualScript/Nodes/VisualScriptEventNode.h>
#include <VisualScript/Nodes/VisualScriptFunctionNode.h>
#include <VisualScript/Nodes/VisualScriptGetVarNode.h>
#include <VisualScript/Nodes/VisualScriptSetVarNode.h>
#include <VisualScript/Nodes/VisualScriptGetMemberNode.h>
#include <VisualScript/Nodes/VisualScriptSetMemberNode.h>
#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptExecutor.h>
#include <VisualScript/VisualScriptNode.h>
#include <VisualScript/VisualScriptPin.h>
#include <VisualScript/VisualScriptEvents.h>

using namespace DAVA;

namespace VisualScriptTestDetils
{
class TestObject
{
public:
    DAVA_REFLECTION(TestObject);

    void SetPositionVal(Vector3 vec_)
    {
        vec = vec_;
    }

    void SetPositionConstVal(const Vector3 vec_)
    {
        vec = vec_;
    }

    void SetPositionConstValRef(const Vector3& vec_)
    {
        vec = vec_;
    }

    Vector3 vec;
};

DAVA_REFLECTION_IMPL(TestObject)
{
    ReflectionRegistrator<TestObject>::Begin()
    .Method("SetPositionVal", &TestObject::SetPositionVal)[M::ArgNames("a")]
    .Method("SetPositionConstVal", &TestObject::SetPositionConstVal)[M::ArgNames("a")]
    .Method("SetPositionConstValRef", &TestObject::SetPositionConstValRef)[M::ArgNames("a")]
    .End();
};

class VisualScriptTestData : public ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION(FormulaTestData);

public:
    int inputValue0 = 5;
    int inputValue1 = 6;
    int outputValue = 7;

    int intVal = 42;
    int t = 1;
    TestObject* testObject = nullptr;
    Vector3 testVector = Vector3(-1.0, 1.0, 2.0);
    Vector3 testVector2 = Vector3(1.0, 1.0, 2.0);

    VisualScriptTestData()
    {
        testObject = new TestObject();
    }

    static int sum(int a, int b)
    {
        return a + b;
    }

    static int mul(int a, int b)
    {
        return a * b;
    };

    bool conditionValue = true;
};

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptTestData)
{
    ReflectionRegistrator<VisualScriptTestData>::Begin()
    .Field("inputValue0", &VisualScriptTestData::inputValue0)
    .Field("inputValue1", &VisualScriptTestData::inputValue1)
    .Field("outputValue", &VisualScriptTestData::outputValue)
    .Field("testVector", &VisualScriptTestData::testVector)
    .Field("testVector2", &VisualScriptTestData::testVector2)
    .Field("val", &VisualScriptTestData::intVal)
    .Field("conditionValue", &VisualScriptTestData::conditionValue)
    .Field("testObject", &VisualScriptTestData::testObject)
    .Method("sum", &VisualScriptTestData::sum)[M::ArgNames("a", "b")]
    .Method("mul", &VisualScriptTestData::mul)[M::ArgNames("a", "b")]
    .End();
};

class VisualScriptTestDataEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(VisualScriptTestDataEvent, VisualScriptEvent);
    VisualScriptTestDataEvent() = default;

    VisualScriptTestData* data = nullptr;
    static const FastName NAME;
};

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptTestDataEvent)
{
    ReflectionRegistrator<VisualScriptTestDataEvent>::Begin()
    .ConstructorByPointer()
    .Field("data", &VisualScriptTestDataEvent::data)
    .End();
}
const FastName VisualScriptTestDataEvent::NAME = FastName("VisualScriptTestDataEvent");

DAVA_TESTCLASS (VisualScriptTest)
{
    /*
         + Simple graph calculation test
         - Complex graph execution test
         - Flow control test
         - Types test
         - ObjectPtrs test
         - Entities + Components complex test
         - Serialisation/deserialisation test
     */

    DAVA_TEST (VisualScriptFirstTest)
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptTestData);

        VisualScriptTestData data;
        Reflection ref = DAVA::Reflection::Create(&data);
        // Reflection
        try
        {
            VisualScript* script = new VisualScript();
            VisualScriptGetVarNode* varIn1 = new VisualScriptGetVarNode(ref, FastName("inputValue0"));
            VisualScriptGetVarNode* varIn2 = new VisualScriptGetVarNode(ref, FastName("inputValue1"));

            VisualScriptFunctionNode* function = new VisualScriptFunctionNode(FastName("VisualScriptTestData"), FastName("sum"));
            function->GetDataInputPin(0)->Connect(varIn1->GetDataOutputPin(0));
            function->GetDataInputPin(1)->Connect(varIn2->GetDataOutputPin(0));

            TEST_VERIFY(VisualScriptPin::IsConnected(function->GetDataInputPin(0), varIn1->GetDataOutputPin(0)));
            TEST_VERIFY(VisualScriptPin::IsConnected(varIn1->GetDataOutputPin(0), function->GetDataInputPin(0)));

            TEST_VERIFY(VisualScriptPin::IsConnected(varIn2->GetDataOutputPin(0), function->GetDataInputPin(1)));
            TEST_VERIFY(VisualScriptPin::IsConnected(function->GetDataInputPin(1), varIn2->GetDataOutputPin(0)));

            VisualScriptSetVarNode* varOut = new VisualScriptSetVarNode(ref, FastName("outputValue"));
            function->GetDataOutputPin(0)->Connect(varOut->GetDataInputPin(0));

            TEST_VERIFY(VisualScriptPin::IsConnected(function->GetDataOutputPin(0), varOut->GetDataInputPin(0)));
            TEST_VERIFY(VisualScriptPin::IsConnected(varOut->GetDataInputPin(0), function->GetDataOutputPin(0)));

            VisualScriptExecutor executor;
            executor.Execute(varOut->GetExecInputPin(0));

            TEST_VERIFY(data.outputValue == data.inputValue0 + data.inputValue1);

            SafeDelete(varIn1);
            SafeDelete(varIn2);
            SafeDelete(function);
            SafeDelete(varOut);
            SafeDelete(script);
        }
        catch (Exception& exception)
        {
            VSLogger_Error(exception.what());
        }
    }

    DAVA_TEST (VisualScriptSecondTest)
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TestObject);

        VisualScriptTestData data;
        Reflection ref = DAVA::Reflection::Create(&data);

        try
        {
            VisualScript* script = new VisualScript();

            VisualScriptGetVarNode* getNode = new VisualScriptGetVarNode(ref, FastName("testObject"));
            VisualScriptGetVarNode* getTestVector = new VisualScriptGetVarNode(ref, FastName("testVector"));
            VisualScriptFunctionNode* setPositionValNode = new VisualScriptFunctionNode(FastName("TestObject"), FastName("SetPositionVal"));
            VisualScriptFunctionNode* setPositionConstValNode = new VisualScriptFunctionNode(FastName("TestObject"), FastName("SetPositionConstVal"));
            VisualScriptFunctionNode* setPositionConstValRefNode = new VisualScriptFunctionNode(FastName("TestObject"), FastName("SetPositionConstValRef"));

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionValNode->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector->GetDataOutputPin(0)->Connect(setPositionValNode->GetDataInputPin(1)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionConstValNode->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector->GetDataOutputPin(0)->Connect(setPositionConstValNode->GetDataInputPin(1)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionConstValRefNode->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector->GetDataOutputPin(0)->Connect(setPositionConstValRefNode->GetDataInputPin(1)) == true);

            VisualScriptExecutor executor;

            DVASSERT(data.testObject->vec != data.testVector);
            executor.Execute(setPositionValNode->GetExecInputPin(0));
            DVASSERT(data.testObject->vec == data.testVector);

            data.testObject->vec += Vector3(1.0f, 1.0f, 1.0f);
            DVASSERT(data.testObject->vec != data.testVector);
            executor.Execute(setPositionConstValNode->GetExecInputPin(0));
            DVASSERT(data.testObject->vec == data.testVector);

            data.testObject->vec += Vector3(1.0f, 1.0f, 1.0f);
            TEST_VERIFY(data.testObject->vec != data.testVector);
            executor.Execute(setPositionConstValRefNode->GetExecInputPin(0));
            TEST_VERIFY(data.testObject->vec == data.testVector);

            SafeDelete(getNode);
            SafeDelete(getTestVector);
            SafeDelete(setPositionValNode);
            SafeDelete(setPositionConstValNode);
            SafeDelete(setPositionConstValRefNode);
            SafeDelete(script);
        }
        catch (Exception& exception)
        {
            VSLogger_Error(exception.what());
        }
    }

    DAVA_TEST (VisualScriptBranchTest)
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TestObject);

        VisualScriptTestData data;
        Reflection ref = DAVA::Reflection::Create(&data);
        try
        {
            VisualScriptGetVarNode* getCondition = new VisualScriptGetVarNode(ref, FastName("conditionValue"));
            VisualScriptBranchNode* branchNode = new VisualScriptBranchNode();

            VisualScriptGetVarNode* getNode = new VisualScriptGetVarNode(ref, FastName("testObject"));
            VisualScriptGetVarNode* getTestVector = new VisualScriptGetVarNode(ref, FastName("testVector"));
            VisualScriptGetVarNode* getTestVector2 = new VisualScriptGetVarNode(ref, FastName("testVector2"));
            VisualScriptFunctionNode* setPositionNodeTrue = new VisualScriptFunctionNode(FastName("TestObject"), FastName("SetPositionVal"));
            VisualScriptFunctionNode* setPositionNodeFalse = new VisualScriptFunctionNode(FastName("TestObject"), FastName("SetPositionVal"));

            TEST_VERIFY(branchNode->GetExecOutputPin(0)->Connect(setPositionNodeTrue->GetExecInputPin(0)) == true);
            TEST_VERIFY(branchNode->GetExecOutputPin(1)->Connect(setPositionNodeFalse->GetExecInputPin(0)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionNodeTrue->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector->GetDataOutputPin(0)->Connect(setPositionNodeTrue->GetDataInputPin(1)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionNodeFalse->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector2->GetDataOutputPin(0)->Connect(setPositionNodeFalse->GetDataInputPin(1)) == true);

            TEST_VERIFY(getCondition->GetDataOutputPin(0)->Connect(branchNode->GetDataInputPin(0)) == true);

            VisualScriptExecutor executor;
            DVASSERT(data.testObject->vec != data.testVector);
            data.conditionValue = true;
            executor.Execute(branchNode->GetExecInputPin(0));
            DVASSERT(data.testObject->vec == data.testVector);

            data.conditionValue = false;
            executor.Execute(branchNode->GetExecInputPin(0));
            DVASSERT(data.testObject->vec == data.testVector2);

            SafeDelete(getNode);
            SafeDelete(branchNode);
            SafeDelete(getCondition);
            SafeDelete(getTestVector);
            SafeDelete(getTestVector2);
            SafeDelete(setPositionNodeTrue);
            SafeDelete(setPositionNodeFalse);
        }
        catch (Exception& exception)
        {
            VSLogger_Error(exception.what());
            TEST_VERIFY(0 && "Exception thrown");
        }
    }

    DAVA_TEST (VisualScriptSaveLoadTest)
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TestObject);
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptTestDataEvent);
        VisualScriptTestData data;

        try
        {
            std::shared_ptr<VisualScript> script = std::make_shared<VisualScript>();

            auto getCondition = script->CreateNode<VisualScriptGetMemberNode>(FastName("VisualScriptTestData"), FastName("conditionValue"));
            auto branchNode = script->CreateNode<VisualScriptBranchNode>();

            auto eventNode = script->CreateNode<VisualScriptEventNode>(FastName("VisualScriptTestDataEvent"));
            auto getNode = script->CreateNode<VisualScriptGetMemberNode>(FastName("VisualScriptTestData"), FastName("testObject"));
            auto getTestVector = script->CreateNode<VisualScriptGetMemberNode>(FastName("VisualScriptTestData"), FastName("testVector"));
            auto getTestVector2 = script->CreateNode<VisualScriptGetMemberNode>(FastName("VisualScriptTestData"), FastName("testVector2"));
            auto setPositionNodeTrue = script->CreateNode<VisualScriptFunctionNode>(FastName("TestObject"), FastName("SetPositionVal"));
            auto setPositionNodeFalse = script->CreateNode<VisualScriptFunctionNode>(FastName("TestObject"), FastName("SetPositionVal"));

            TEST_VERIFY(eventNode->GetExecOutputPin(0)->Connect(branchNode->GetExecInputPin(0)) == true);
            TEST_VERIFY(eventNode->GetDataOutputPin(0)->Connect(getNode->GetDataInputPin(0)) == true);
            TEST_VERIFY(eventNode->GetDataOutputPin(0)->Connect(getCondition->GetDataInputPin(0)) == true);

            TEST_VERIFY(branchNode->GetExecOutputPin(0)->Connect(setPositionNodeTrue->GetExecInputPin(0)) == true);
            TEST_VERIFY(branchNode->GetExecOutputPin(1)->Connect(setPositionNodeFalse->GetExecInputPin(0)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionNodeTrue->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector->GetDataOutputPin(0)->Connect(setPositionNodeTrue->GetDataInputPin(1)) == true);
            TEST_VERIFY(eventNode->GetDataOutputPin(0)->Connect(getTestVector->GetDataInputPin(0)) == true);

            TEST_VERIFY(getNode->GetDataOutputPin(0)->Connect(setPositionNodeFalse->GetDataInputPin(0)) == true);
            TEST_VERIFY(getTestVector2->GetDataOutputPin(0)->Connect(setPositionNodeFalse->GetDataInputPin(1)) == true);
            TEST_VERIFY(eventNode->GetDataOutputPin(0)->Connect(getTestVector2->GetDataInputPin(0)) == true);

            TEST_VERIFY(getCondition->GetDataOutputPin(0)->Connect(branchNode->GetDataInputPin(0)) == true);

            script->Compile();

            auto Execute = [&](std::shared_ptr<VisualScript>& script, VisualScriptTestData& data) {
                VisualScriptTestDataEvent event;
                event.data = &data;
                script->Execute(VisualScriptTestDataEvent::NAME, Reflection::Create(&event));
            };

            TEST_VERIFY(data.testVector != data.testVector2);

            data.conditionValue = true;
            Execute(script, data);
            TEST_VERIFY(data.testObject->vec == data.testVector);

            data.conditionValue = false;
            Execute(script, data);
            TEST_VERIFY(data.testObject->vec == data.testVector2);
            script->Save("~doc:/test.nodegraph");

            std::shared_ptr<VisualScript> scriptLoad = std::make_shared<VisualScript>();
            scriptLoad->Load("~doc:/test.nodegraph");

            data.conditionValue = true;
            Execute(scriptLoad, data);
            TEST_VERIFY(data.testObject->vec == data.testVector);

            data.conditionValue = false;
            Execute(scriptLoad, data);
            TEST_VERIFY(data.testObject->vec == data.testVector2);
        }
        catch (Exception& exception)
        {
            VSLogger_Error(exception.what());
            TEST_VERIFY(0 && "Exception thrown");
        }
    };

    DAVA_TEST (VisualScriptForTest)
    {
        VisualScriptTestData data;

        {};
    };

    DAVA_TEST (VisualScriptWhileTest)
    {
        VisualScriptTestData data;

        {};
    };
};
};
