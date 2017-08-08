#include "Classes/Application/REGlobal.h"
#include "Classes/Application/ReflectionExtensions.h"

#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectResources.h"
#include "Classes/Project/ProjectManagerData.h"

#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include "Classes/CommandLine/Private/CommandLineModuleTestUtils.h"

#include "Classes/Settings/Settings.h"
#include "Classes/Settings/SettingsManager.h"

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/MockDefine.h>
#include <TArc/Testing/MockListener.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/Core/ContextAccessor.h>

#include <Base/Any.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>

#include <QMainWindow>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTest>

#include <gmock/gmock.h>

namespace SMTest
{
const DAVA::String testFolder = DAVA::String("~doc:/Test/");
const DAVA::String testProjectPath = DAVA::String("~doc:/Test/SceneManagerTest/");
const DAVA::String testScenePath = DAVA::String("~doc:/Test/SceneManagerTest/DataSource/3d/Maps/scene.sc2");
const DAVA::FastName allEntitiesName = DAVA::FastName("AllComponentsEntity");

class ProjectManagerDummyModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        // prepare test environment
        {
            CommandLineModuleTestUtils::CreateTestFolder(testFolder);
            CommandLineModuleTestUtils::CreateProjectInfrastructure(testProjectPath);
        }

        projectResources.reset(new ProjectResources(GetAccessor()));
        projectResources->LoadProject(testProjectPath);
    }

    ~ProjectManagerDummyModule() override
    {
        CommandLineModuleTestUtils::ClearTestFolder(testFolder);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ProjectManagerDummyModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ProjectManagerDummyModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

    std::unique_ptr<ProjectResources> projectResources;
};
}

DAVA_TARC_TESTCLASS(SceneManagerModuleTests)
{
    DAVA::Vector<const DAVA::ReflectedType*> componentTypes;
    void InitComponentDerivedTypes(const DAVA::Type* type)
    {
        using namespace DAVA;

        const TypeInheritance* inheritance = type->GetInheritance();
        Vector<TypeInheritance::Info> derivedTypes = inheritance->GetDerivedTypes();
        for (const TypeInheritance::Info& derived : derivedTypes)
        {
            const ReflectedType* refType = ReflectedTypeDB::GetByType(derived.type);
            if (refType == nullptr)
            {
                continue;
            }

            const std::unique_ptr<ReflectedMeta>& meta = refType->GetStructure()->meta;
            if (meta != nullptr && (nullptr != meta->GetMeta<M::CantBeCreatedManualyComponent>()))
            {
                continue;
            }

            if (refType->GetCtor(derived.type->Pointer()) != nullptr)
            {
                componentTypes.emplace_back(refType);
            }

            InitComponentDerivedTypes(derived.type);
        }
    }

    class BaseTestListener : public DAVA::TArc::DataListener
    {
    public:
        bool testSucceed = false;
    };

public:
    SceneManagerModuleTests()
    {
        InitComponentDerivedTypes(DAVA::Type::Instance<DAVA::Component>());
    }

private:
    class NewSceneListener : public BaseTestListener
    {
    public:
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        void OnDataChanged(const DAVA::TArc::DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields) override
        {
            TEST_VERIFY(fields.empty());
            if (w.HasData())
            {
                using namespace DAVA;

                SceneData* data = accessor->GetActiveContext()->GetData<SceneData>();
                TEST_VERIFY(data != nullptr);

                FilePath scenePath = data->GetScenePath();
                TEST_VERIFY(scenePath.GetFilename() == "newscene1.sc2");
                FileSystem* fs = GetEngineContext()->fileSystem;
                TEST_VERIFY(fs->Exists(scenePath) == false);

                SceneData::TSceneType scene = data->GetScene();
                TEST_VERIFY(scene);
                scene->Update(0.16f);

                TEST_VERIFY(scene->GetChildrenCount() == 2);
                TEST_VERIFY(scene->FindByName("editor.camera-light") != nullptr);
                TEST_VERIFY(scene->FindByName("editor.debug-camera") != nullptr);

                testSucceed = true;
            }
        }
    };

    DAVA_TEST (CreateNewEmptyScene)
    {
        NewSceneListener listener;
        listener.accessor = GetAccessor();

        DAVA::TArc::DataWrapper wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<SceneData>());
        wrapper.SetListener(&listener);

        TEST_VERIFY(wrapper.HasData() == false);
        if (wrapper.HasData() == false)
        {
            InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);
        }

        TEST_VERIFY(listener.testSucceed);
    }

    DAVA_TEST (GenerateSceneDataAndSaveScene)
    {
        SceneData* data = GetAccessor()->GetActiveContext()->GetData<SceneData>();
        TEST_VERIFY(data != nullptr);
        if (data != nullptr)
        {
            using namespace DAVA;

            SceneData::TSceneType scene = data->GetScene();
            TEST_VERIFY(scene);
            scene->Update(0.16f);

            ScopedPtr<Entity> entity(new Entity());
            entity->SetName(SMTest::allEntitiesName);
            scene->AddNode(entity);

            { //Add generated components
                for (const ReflectedType* cType : componentTypes)
                {
                    Any newComponent = cType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
                    Component* component = newComponent.Cast<Component*>();

                    if (cType->GetPermanentName() == "SlotComponent")
                    { // to prevent serialization crash
                        SlotComponent* slot = static_cast<SlotComponent*>(component);
                        slot->SetSlotName(FastName("slotName"));
                        slot->SetJointName(FastName("jointName"));
                    }

                    entity->AddComponent(component);
                }
            }

            CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(SMTest::testScenePath, scene.Get());
        }

        CloseActiveScene();
    }

    class OpenSavedSceneListener : public BaseTestListener
    {
    public:
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::Vector<const DAVA::ReflectedType*> componentTypes;
        void OnDataChanged(const DAVA::TArc::DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields) override
        {
            TEST_VERIFY(fields.empty());
            if (w.HasData())
            {
                using namespace DAVA;

                SceneData* data = accessor->GetActiveContext()->GetData<SceneData>();
                TEST_VERIFY(data != nullptr);

                FilePath scenePath = data->GetScenePath();
                TEST_VERIFY(scenePath == SMTest::testScenePath);

                SceneData::TSceneType scene = data->GetScene();
                TEST_VERIFY(scene);
                scene->Update(0.16f);

                Entity* entity = scene->FindByName(SMTest::allEntitiesName);
                TestEntityWithComponents(entity);

                testSucceed = true;
            }
        }

        void TestEntityWithComponents(DAVA::Entity* entity)
        {
            TEST_VERIFY(entity != nullptr);
            if (entity != nullptr)
            {
                using namespace DAVA;

                for (const ReflectedType* cType : componentTypes)
                {
                    Any newComponent = cType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
                    Component* component = newComponent.Cast<Component*>();

                    TEST_VERIFY(entity->GetComponentCount(component->GetType()) > 0);
                    if (component->GetType() == Component::SLOT_COMPONENT)
                    { // to prevent serialization crash
                        SlotComponent* slot = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT));
                        if (slot != nullptr)
                        {
                            TEST_VERIFY(slot->GetSlotName() == FastName("slotName"));
                            TEST_VERIFY(slot->GetJointName() == FastName("jointName"));
                        }
                    }
                    delete component;
                }
            }
        }
    };

    DAVA_TEST (OpenSavedScene)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        OpenSavedSceneListener listener;
        listener.accessor = GetAccessor();
        listener.componentTypes = componentTypes;

        DataWrapper wrapper = GetAccessor()->CreateWrapper(ReflectedTypeDB::Get<SceneData>());
        wrapper.SetListener(&listener);

        TEST_VERIFY(wrapper.HasData() == false);
        if (wrapper.HasData() == false)
        {
            InvokeOperation(REGlobal::OpenSceneOperation.ID, FilePath(SMTest::testScenePath));

            TEST_VERIFY(wrapper.HasData() == true);

            { //add scene
                InvokeOperation(REGlobal::AddSceneOperation.ID, FilePath(SMTest::testScenePath));

                SceneData* data = GetAccessor()->GetActiveContext()->GetData<SceneData>();
                TEST_VERIFY(data != nullptr);
                SceneData::TSceneType scene = data->GetScene();
                TEST_VERIFY(scene);
                scene->Update(0.16f);

                scene->SetName(FastName("OriginalScene"));
                FastName loadedEntityName = FastName(FilePath(SMTest::testScenePath).GetFilename());
                Entity* loadedEntity = scene->FindByName(loadedEntityName);
                TEST_VERIFY(loadedEntity != nullptr);
                if (loadedEntity != nullptr)
                {
                    Entity* testedEntity = loadedEntity->FindByName(SMTest::allEntitiesName);
                    listener.TestEntityWithComponents(testedEntity);
                }
            }

            {
                InvokeOperation(REGlobal::SaveCurrentScene.ID);
            }
        }
        TEST_VERIFY(listener.testSucceed);

        CloseActiveScene();
    }

    DAVA_TEST (OpenSavedModifiedScene)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;

        OpenSavedSceneListener listener;
        listener.accessor = GetAccessor();
        listener.componentTypes = componentTypes;

        DataWrapper wrapper = GetAccessor()->CreateWrapper(ReflectedTypeDB::Get<SceneData>());
        wrapper.SetListener(&listener);

        TEST_VERIFY(wrapper.HasData() == false);
        if (wrapper.HasData() == false)
        {
            InvokeOperation(REGlobal::OpenSceneOperation.ID, FilePath(SMTest::testScenePath));

            TEST_VERIFY(wrapper.HasData() == true);

            { //Test scene
                SceneData* data = GetAccessor()->GetActiveContext()->GetData<SceneData>();
                TEST_VERIFY(data != nullptr);
                SceneData::TSceneType scene = data->GetScene();
                TEST_VERIFY(scene);
                scene->Update(0.16f);

                Vector<Entity*> foundEntities;
                scene->GetChildEntitiesWithCondition(foundEntities, [](Entity* entity)
                                                     {
                                                         return (entity->GetName() == SMTest::allEntitiesName);
                                                     });

                TEST_VERIFY(foundEntities.size() == 2);

                for (Entity* entity : foundEntities)
                {
                    listener.TestEntityWithComponents(entity);
                }
            }
        }
        TEST_VERIFY(listener.testSucceed);

        CloseActiveScene();
    }

    DAVA_TEST (OpenResentScene)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        using namespace ::testing;

        OpenSavedSceneListener listener;
        listener.accessor = GetAccessor();
        listener.componentTypes = componentTypes;

        DataWrapper openSceneWrapper = GetAccessor()->CreateWrapper(ReflectedTypeDB::Get<SceneData>());
        openSceneWrapper.SetListener(&listener);

        TEST_VERIFY(openSceneWrapper.HasData() == false);
        if (openSceneWrapper.HasData() == false)
        {
            bool oldValue = SettingsManager::GetValue(Settings::General_OpenLastSceneOnLaunch).AsBool();
            SettingsManager::SetValue(Settings::General_OpenLastSceneOnLaunch, VariantType(true));

            InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);

            SettingsManager::SetValue(Settings::General_OpenLastSceneOnLaunch, VariantType(oldValue));
        }

        TEST_VERIFY(listener.testSucceed);
        CloseActiveScene();
    }

    DAVA_TEST (OpenResentSceneFromMenu)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        using namespace ::testing;

        OpenSavedSceneListener listener;
        listener.accessor = GetAccessor();
        listener.componentTypes = componentTypes;

        DataWrapper openSceneWrapper = GetAccessor()->CreateWrapper(ReflectedTypeDB::Get<SceneData>());
        openSceneWrapper.SetListener(&listener);

        TEST_VERIFY(openSceneWrapper.HasData() == false);
        if (openSceneWrapper.HasData() == false)
        {
            QWidget* wnd = GetWindow(DAVA::TArc::mainWindowKey);
            QMainWindow* mainWnd = qobject_cast<QMainWindow*>(wnd);
            TEST_VERIFY(wnd != nullptr);

            QMenuBar* menu = mainWnd->menuBar();
            QMenu* fileMenu = menu->findChild<QMenu*>(MenuItems::menuFile);

            QList<QAction*> actions = fileMenu->actions();
            TEST_VERIFY(actions.size() > 0);

            QAction* resentSceneAction = *actions.rbegin();
            TEST_VERIFY(resentSceneAction->text() == QString::fromStdString(FilePath(SMTest::testScenePath).GetAbsolutePathname()));
            resentSceneAction->triggered(false);

            testCompleted = false;
            EXPECT_CALL(*this, AfterWrappersSync())
            .WillOnce(Return())
            .WillOnce(Invoke([this, &listener]() {

                testCompleted = true;

                TEST_VERIFY(listener.testSucceed);
                CloseActiveScene();
            }))
            .WillRepeatedly(Return());
        }
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    bool testCompleted = true;
    bool TestComplete(const DAVA::String& testName) const override
    {
        return testCompleted;
    }

    class CloseSceneListener : public BaseTestListener
    {
    public:
        void OnDataChanged(const DAVA::TArc::DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields) override
        {
            TEST_VERIFY(fields.empty());
            TEST_VERIFY(w.HasData() == false);

            testSucceed = true;
        }
    };

    void CloseActiveScene()
    {
        CloseSceneListener listener;
        DAVA::TArc::DataWrapper wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<SceneData>());
        wrapper.SetListener(&listener);

        TEST_VERIFY(wrapper.HasData());
        if (wrapper.HasData())
        {
            InvokeOperation(REGlobal::CloseAllScenesOperation.ID, false);
        }

        TEST_VERIFY(listener.testSucceed);
    }

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ReflectionExtensionsModule)
    DECLARE_TESTED_MODULE(SMTest::ProjectManagerDummyModule)
    DECLARE_TESTED_MODULE(SceneManagerModule)
    END_TESTED_MODULES()
};
