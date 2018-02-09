#pragma once

#include "Infrastructure/BaseScreen.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UITextField;
class UIStaticText;
class Scene;
}

class TestBed;
class AnyPerformanceTest : public BaseScreen
{
public:
    template <typename T>
    struct SceneField
    {
        DAVA::String path;
        T* valuePtr;
        T lastValue;
    };

    AnyPerformanceTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

    DAVA::UITextField* testCount;
    DAVA::UIStaticText* resultCreate;
    DAVA::UIStaticText* resultGetSet;
    DAVA::UIStaticText* resultWalkScene;

    DAVA::uint64 GetLoopCount();
    void SetResult(DAVA::UIStaticText*, DAVA::uint64 ms);
    void OnCreateTest(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnGetSetTest(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnWalkScene(DAVA::BaseObject* sender, void* data, void* callerData);

    DAVA::Scene* testScene = nullptr;

    DAVA::Vector<SceneField<bool>> sceneFieldsBool;
    DAVA::Vector<SceneField<DAVA::FastName>> sceneFieldsFastName;
    DAVA::Vector<SceneField<DAVA::String>> sceneFieldsString;
    DAVA::Vector<SceneField<DAVA::FilePath>> sceneFieldsFilePath;
    DAVA::Vector<SceneField<DAVA::int32>> sceneFieldsInt32;
    DAVA::Vector<SceneField<DAVA::uint32>> sceneFieldsUint32;
    DAVA::Vector<SceneField<DAVA::int64>> sceneFieldsInt64;
    DAVA::Vector<SceneField<DAVA::uint64>> sceneFieldsUint64;
    DAVA::Vector<SceneField<DAVA::float32>> sceneFieldsFloat32;
    DAVA::Vector<SceneField<DAVA::Matrix4>> sceneFieldsMatrix4;
    DAVA::Vector<SceneField<DAVA::Matrix3>> sceneFieldsMatrix3;
    DAVA::Vector<SceneField<DAVA::Vector4>> sceneFieldsVector4;
    DAVA::Vector<SceneField<DAVA::Vector3>> sceneFieldsVector3;
    DAVA::Vector<SceneField<DAVA::Color>> sceneFieldsColor;
};
