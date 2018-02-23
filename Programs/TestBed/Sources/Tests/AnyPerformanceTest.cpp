#include "Base/Any.h"
#include "Tests/AnyPerformanceTest.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"
#include "Scene3D/SceneFileV2.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectionSerializer.h"

#include <fstream>

using namespace DAVA;

AnyPerformanceTest::AnyPerformanceTest(TestBed& app)
    : BaseScreen(app, "AnyPerformanceTest")
{
}

void AnyPerformanceTest::LoadResources()
{
    using namespace DAVA;

    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/TestBed/Fonts/korinna.ttf"));
    font->SetSize(14);

    float y = 10;
    float h = 35;
    float dy = h + 7;

    ScopedPtr<UIStaticText> uitext(new UIStaticText(Rect(10, y, 100, h)));
    uitext->SetText(L"Loop count:");
    uitext->SetFont(font);
    AddControl(uitext);

    testCount = new UITextField(Rect(110, y, 100, h));
    testCount->GetOrCreateComponent<UIDebugRenderComponent>();
    testCount->SetFont(font);
    testCount->SetInputEnabled(true);
    testCount->SetText(L"1000");
    testCount->GetOrCreateComponent<UIFocusComponent>();
    AddControl(testCount);

    y += dy;
    ScopedPtr<UIButton> testCreate(new UIButton(Rect(10, y, 200, h)));
    testCreate->GetOrCreateComponent<UIDebugRenderComponent>();
    testCreate->SetStateFont(0xFF, font);
    testCreate->SetStateFontColor(0xFF, Color::White);
    testCreate->SetStateText(0xFF, L"CreateTest");
    testCreate->SetStateFontColor(UIButton::STATE_PRESSED_INSIDE, Color(0, 1.0f, 1.0f, 1.0f));
    testCreate->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &AnyPerformanceTest::OnCreateTest));
    resultCreate = new UIStaticText(Rect(210, y, 400, h));
    resultCreate->SetFont(font);
    AddControl(testCreate);
    AddControl(resultCreate);

    y += dy;
    ScopedPtr<UIButton> testGetSet(new UIButton(Rect(10, y, 200, h)));
    testGetSet->GetOrCreateComponent<UIDebugRenderComponent>();
    testGetSet->SetStateFont(0xFF, font);
    testGetSet->SetStateFontColor(0xFF, Color::White);
    testGetSet->SetStateText(0xFF, L"GetSetTest");
    testGetSet->SetStateFontColor(UIButton::STATE_PRESSED_INSIDE, Color(0, 1.0f, 1.0f, 1.0f));
    testGetSet->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &AnyPerformanceTest::OnGetSetTest));
    resultGetSet = new UIStaticText(Rect(210, y, 400, h));
    resultGetSet->SetFont(font);
    AddControl(testGetSet);
    AddControl(resultGetSet);

    y += dy;
    ScopedPtr<UIButton> testWalkScene(new UIButton(Rect(10, y, 200, h)));
    testWalkScene->GetOrCreateComponent<UIDebugRenderComponent>();
    testWalkScene->SetStateFont(0xFF, font);
    testWalkScene->SetStateFontColor(0xFF, Color::White);
    testWalkScene->SetStateText(0xFF, L"WalkScene");
    testWalkScene->SetStateFontColor(UIButton::STATE_PRESSED_INSIDE, Color(0, 1.0f, 1.0f, 1.0f));
    testWalkScene->AddEvent(UIButton::EVENT_TOUCH_DOWN, Message(this, &AnyPerformanceTest::OnWalkScene));
    resultWalkScene = new UIStaticText(Rect(210, y, 400, h));
    resultWalkScene->SetFont(font);
    AddControl(testWalkScene);
    AddControl(resultWalkScene);

    testScene = new Scene();
    testScene->LoadScene("~res:/3d/Maps/12_malinovka_ma/12_malinovka_ma.sc2");

    Logger::Info("Starting scene reflect");

    {
        Reflection sceneRef = Reflection::Create(&this->testScene);

        std::set<void*> pointers;
        std::function<void(const Reflection& ref)> fn;

        fn = [&fn, &pointers, this](const Reflection& ref)
        {
            const Type* valueType = ref.GetValueType()->Decay();

            if (valueType->IsPointer())
            {
                void* pointer = ref.GetValue().Get<void*>();
                if (pointers.find(pointer) == pointers.end())
                {
                    pointers.insert(pointer);
                }
                else
                {
                    return;
                }
            }

            /*
                DAVA::Vector<SceneField<bool>> sceneFieldsBool;
                DAVA::Vector<SceneField<DAVA::FastName>> sceneFieldsFastName;
                DAVA::Vector<SceneField<DAVA::String>> sceneFieldsString;
                DAVA::Vector<SceneField<DAVA::int32>> sceneFieldsInt32;
                DAVA::Vector<SceneField<DAVA::uint32>> sceneFieldsUint32;
                DAVA::Vector<SceneField<DAVA::float32>> sceneFieldsFloat32;
                DAVA::Vector<SceneField<DAVA::Matrix4>> sceneFieldsMatrix4;
                DAVA::Vector<SceneField<DAVA::Matrix3>> sceneFieldsMatrix3;
                DAVA::Vector<SceneField<DAVA::Vector4>> sceneFieldsVector4;
                DAVA::Vector<SceneField<DAVA::Vector3>> sceneFieldsVector3;
                DAVA::Vector<SceneField<DAVA::Color>> sceneFieldsColor;
            */

            if (valueType->Is<bool>())
            {
                SceneField<bool> f;
                f.valuePtr = static_cast<bool*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsBool.push_back(f);
            }
            else if (valueType->Is<FastName>())
            {
                SceneField<FastName> f;
                f.valuePtr = static_cast<FastName*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsFastName.push_back(f);
            }
            else if (valueType->Is<String>())
            {
                //SceneField<String> f;
                //f.valuePtr = static_cast<String*>(ref.GetValueObject().GetVoidPtr());
                //if (nullptr != f.valuePtr) sceneFieldsString.push_back(f);
            }
            else if (valueType->Is<FilePath>())
            {
                SceneField<FilePath> f;
                f.valuePtr = static_cast<FilePath*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsFilePath.push_back(f);
            }
            else if (valueType->Is<int32>())
            {
                SceneField<int32> f;
                f.valuePtr = static_cast<int32*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsInt32.push_back(f);
            }
            else if (valueType->Is<uint32>())
            {
                SceneField<uint32> f;
                f.valuePtr = static_cast<uint32*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsUint32.push_back(f);
            }
            else if (valueType->Is<int64>())
            {
                SceneField<int64> f;
                f.valuePtr = static_cast<int64*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsInt64.push_back(f);
            }
            else if (valueType->Is<uint64>())
            {
                SceneField<uint64> f;
                f.valuePtr = static_cast<uint64*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsUint64.push_back(f);
            }
            else if (valueType->Is<float32>())
            {
                SceneField<float32> f;
                f.valuePtr = static_cast<float32*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsFloat32.push_back(f);
            }
            else if (valueType->Is<Matrix4>())
            {
                SceneField<Matrix4> f;
                f.valuePtr = static_cast<Matrix4*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsMatrix4.push_back(f);
            }
            else if (valueType->Is<Matrix3>())
            {
                SceneField<Matrix3> f;
                f.valuePtr = static_cast<Matrix3*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsMatrix3.push_back(f);
            }
            else if (valueType->Is<Vector4>())
            {
                SceneField<Vector4> f;
                f.valuePtr = static_cast<Vector4*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsVector4.push_back(f);
            }
            else if (valueType->Is<Vector3>())
            {
                SceneField<Vector3> f;
                f.valuePtr = static_cast<Vector3*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsVector3.push_back(f);
            }
            else if (valueType->Is<Color>())
            {
                SceneField<Color> f;
                f.valuePtr = static_cast<Color*>(ref.GetValueObject().GetVoidPtr());
                if (nullptr != f.valuePtr)
                    sceneFieldsColor.push_back(f);
            }
            else if (!ref.HasFields() && !valueType->IsPointer() && !ref.GetFieldsCaps().hasDynamicStruct)
            {
                //DVASSERT(false);
            }
            else
            {
                Vector<Reflection::Field> fields = ref.GetFields();
                for (size_t i = 0; i < fields.size(); ++i)
                {
                    fn(fields[i].ref);
                }
            }
        };

        fn(sceneRef);
    }
    Logger::Info("Done");

    std::sort(sceneFieldsBool.begin(), sceneFieldsBool.end(), [](const SceneField<bool>& p1, const SceneField<bool>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsFastName.begin(), sceneFieldsFastName.end(), [](const SceneField<FastName>& p1, const SceneField<FastName>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsString.begin(), sceneFieldsString.end(), [](const SceneField<String>& p1, const SceneField<String>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsFilePath.begin(), sceneFieldsFilePath.end(), [](const SceneField<FilePath>& p1, const SceneField<FilePath>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsInt32.begin(), sceneFieldsInt32.end(), [](const SceneField<int32>& p1, const SceneField<int32>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsUint32.begin(), sceneFieldsUint32.end(), [](const SceneField<uint32>& p1, const SceneField<uint32>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsInt64.begin(), sceneFieldsInt64.end(), [](const SceneField<int64>& p1, const SceneField<int64>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsUint64.begin(), sceneFieldsUint64.end(), [](const SceneField<uint64>& p1, const SceneField<uint64>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsFloat32.begin(), sceneFieldsFloat32.end(), [](const SceneField<float32>& p1, const SceneField<float32>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsMatrix4.begin(), sceneFieldsMatrix4.end(), [](const SceneField<Matrix4>& p1, const SceneField<Matrix4>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsMatrix3.begin(), sceneFieldsMatrix3.end(), [](const SceneField<Matrix3>& p1, const SceneField<Matrix3>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsVector4.begin(), sceneFieldsVector4.end(), [](const SceneField<Vector4>& p1, const SceneField<Vector4>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsVector3.begin(), sceneFieldsVector3.end(), [](const SceneField<Vector3>& p1, const SceneField<Vector3>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
    std::sort(sceneFieldsColor.begin(), sceneFieldsColor.end(), [](const SceneField<Color>& p1, const SceneField<Color>& p2) -> bool { return p1.valuePtr < p2.valuePtr; });
}

void AnyPerformanceTest::UnloadResources()
{
    SafeRelease(testScene);

    SafeRelease(testCount);
    SafeRelease(resultCreate);
    SafeRelease(resultGetSet);
    SafeRelease(resultWalkScene);

    BaseScreen::UnloadResources();
}

DAVA::uint64 AnyPerformanceTest::GetLoopCount()
{
    int res = 0;

    auto str = testCount->GetText();
    sscanf(DAVA::UTF8Utils::EncodeToUTF8(str).c_str(), "%u", &res);

    return res;
}

void AnyPerformanceTest::SetResult(DAVA::UIStaticText* st, DAVA::uint64 ns)
{
    double usDouble = static_cast<double>(ns / 1000);
    double msDouble = usDouble / 1000.0;
    st->SetText(DAVA::Format(L"%llu ns (%f ms)", ns, msDouble));
}

void AnyPerformanceTest::OnCreateTest(DAVA::BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;

    const Type* type = nullptr;

    uint64 sz = GetLoopCount();
    uint64 startNs = SystemTimer::GetNs();
    for (uint64 i = 0; i < sz; ++i)
    {
        Any a(i);
        type = a.GetType();
    }
    uint64 endNs = SystemTimer::GetNs();

    Logger::FrameworkDebug("%p", type);
    SetResult(resultCreate, endNs - startNs);
}

void AnyPerformanceTest::OnGetSetTest(DAVA::BaseObject* sender, void* data, void* callerData)
{
    using namespace DAVA;

    Any a(float32(0));

    uint64 sz = GetLoopCount();
    uint64 startNs = SystemTimer::GetNs();
    for (uint64 i = 0; i < sz; ++i)
    {
        float32 v = a.Get<float32>();
        v += static_cast<float32>(i);
        a.Set(v);
    }
    uint64 endNs = SystemTimer::GetNs();

    Logger::FrameworkDebug("%f", a.Get<float32>());
    SetResult(resultGetSet, endNs - startNs);
}

template <typename T>
void Check(Vector<AnyPerformanceTest::SceneField<T>>& vec)
{
    for (size_t i = 0; i < vec.size(); ++i)
    {
        T val = (*vec[i].valuePtr);
        if (vec[i].lastValue != val)
        {
            vec[i].lastValue = val;
        }
    }
}

void AnyPerformanceTest::OnWalkScene(DAVA::BaseObject* sender, void* data, void* callerData)
{
    /*
    struct SceneWalkContext
    {
        std::set<void*> pointers;
        size_t fields = 0;
    };

    std::stringstream s;
    SceneWalkContext context;

    uint64 startNs = SystemTimer::GetNs();
    {
        Reflection sceneRef = Reflection::Create(&this->testScene);
        ReflectionSerializer::Save(s, Any("scene"), sceneRef, &context, [](void* context, const Reflection::Field& field) -> bool {

            SceneWalkContext* ctx = static_cast<SceneWalkContext*>(context);

            ctx->fields++;

            const Type* valueType = field.ref.GetValueType();
            if (valueType->IsPointer())
            {
                void *pointer = field.ref.GetValue().Get<void*>();
                if (nullptr == pointer)
                {
                    return false;
                }
                if (ctx->pointers.find(pointer) == ctx->pointers.end())
                {
                    ctx->pointers.insert(pointer);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            return true;
        });
    }
    uint64 endNs = SystemTimer::GetNs();

    std::ofstream outFile;
    outFile.open("WalkScene.out", std::ios_base::out);
    outFile << s.rdbuf();
    outFile.close();

    Logger::FrameworkDebug("%d fields processed, output size: %d", context.fields, static_cast<int>(s.tellp()));
    this->SetResult(this->resultWalkScene, endNs - startNs);
    */

    uint64 startNs = SystemTimer::GetNs();
    /*
    for (size_t i = 0; i < sceneFields.size(); ++i)
    {
        Any val = sceneFields[i].ref.GetValue();
        if (sceneFields[i].lastValue != val)
        {
            sceneFields[i].lastValue = val;
        }
    }
    */

    Check(sceneFieldsBool);
    Check(sceneFieldsFastName);
    Check(sceneFieldsString);
    Check(sceneFieldsFilePath);
    Check(sceneFieldsInt32);
    Check(sceneFieldsUint32);
    Check(sceneFieldsInt64);
    Check(sceneFieldsUint64);
    Check(sceneFieldsFloat32);
    Check(sceneFieldsMatrix4);
    Check(sceneFieldsMatrix3);
    Check(sceneFieldsVector4);
    Check(sceneFieldsVector3);
    Check(sceneFieldsColor);

    uint64 endNs = SystemTimer::GetNs();
    this->SetResult(this->resultWalkScene, endNs - startNs);
}
