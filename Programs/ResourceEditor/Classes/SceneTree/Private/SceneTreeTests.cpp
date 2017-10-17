#include "Classes/Application/ReflectionExtensions.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/SceneTree/SceneTreeModule.h"
#include "Classes/Selection/Selectable.h"

#include "Classes/MockModules/MockProjectManagerModule.h"

#include <TArc/DataProcessing/AnyQMetaType.h>
#include <TArc/SharedModules/SettingsModule/SettingsModule.h>
#include <TArc/Testing/MockDefine.h>
#include <TArc/Testing/MockListener.h>
#include <TArc/Testing/TArcUnitTests.h>

#include <Base/Any.h>
#include <Scene3D/Components/Controller/WASDControllerComponent.h>

#include <QTreeView>
#include <QVariant>
#include <QtTest>

#include <gmock/gmock.h>

namespace SceneTreeTestsDetails
{
template <typename T>
T* ExtractObject(QModelIndex index)
{
    QVariant sceneVariant = index.data(Qt::UserRole);
    TEST_VERIFY(sceneVariant.canConvert<DAVA::Any>());
    Selectable sceneObj(sceneVariant.value<DAVA::Any>());
    TEST_VERIFY(sceneObj.ContainsObject() == true);

    T* result = nullptr;
    if (sceneObj.CanBeCastedTo<T>() == true)
    {
        result = sceneObj.Cast<T>();
    }

    return result;
}
} // namespace SceneTreeTestsDetails

DAVA_TARC_TESTCLASS(SceneTreeTests)
{
    QTreeView* sceneTreeView = nullptr;

    void MatchEntitiesWithScene()
    {
        using namespace SceneTreeTestsDetails;

        TEST_VERIFY(sceneTreeView != nullptr);
        TEST_VERIFY(sceneTreeView->model() != nullptr);
        QModelIndex rootIndex = sceneTreeView->rootIndex();
        TEST_VERIFY(rootIndex.isValid() == true);

        sceneTreeView->expandAll();

        SceneEditor2* scene = ExtractObject<SceneEditor2>(rootIndex);
        TEST_VERIFY(scene != nullptr);

        SceneEditor2* activeScene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene().Get();

        DAVA::Function<QModelIndex(DAVA::Entity*, QModelIndex index)> checkEntityWithChildren = [&](DAVA::Entity* e, QModelIndex index) {
            DAVA::Entity* entity = ExtractObject<DAVA::Entity>(index);
            TEST_VERIFY(e == entity);

            QModelIndex currentIndex = index;
            for (DAVA::int32 childIndex = 0; childIndex < entity->GetChildrenCount(); ++childIndex)
            {
                DAVA::Entity* child = entity->GetChild(childIndex);
                currentIndex = checkEntityWithChildren(child, sceneTreeView->indexBelow(currentIndex));
            }

            return currentIndex;
        };

        checkEntityWithChildren(activeScene, rootIndex);
    }

    DAVA_TEST (SceneTreeCreationTest)
    {
        sceneTreeView = LookupSingleWidget<QTreeView>(DAVA::TArc::mainWindowKey, QStringLiteral("Scene Tree V2"));
        TEST_VERIFY(sceneTreeView != nullptr);
        TEST_VERIFY(sceneTreeView->model() == nullptr);
        TEST_VERIFY(sceneTreeView->rootIndex().isValid() == false);
        TEST_VERIFY(sceneTreeView->indexBelow(sceneTreeView->rootIndex()).isValid() == false);
    }

    DAVA_TEST (LightAndCameraTest)
    {
        TEST_VERIFY(sceneTreeView != nullptr);

        using namespace DAVA::TArc;
        using namespace ::testing;

        ContextAccessor* accessor = GetAccessor();
        TEST_VERIFY(accessor->GetActiveContext() == nullptr);
        InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);

        DataContext* ctx = accessor->GetActiveContext();
        TEST_VERIFY(ctx != nullptr);

        auto finalStep = [this]() {
            MatchEntitiesWithScene();
            InvokeOperation(REGlobal::CloseAllScenesOperation.ID, false);
        };

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(finalStep))
        .WillOnce(Return());
    }

    DAVA_TEST (SwitchScenesTest)
    {
        TEST_VERIFY(sceneTreeView != nullptr);

        using namespace DAVA::TArc;
        using namespace ::testing;

        ContextAccessor* accessor = GetAccessor();
        TEST_VERIFY(accessor->GetActiveContext() == nullptr);
        InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);

        DataContext* ctx = accessor->GetActiveContext();
        TEST_VERIFY(ctx != nullptr);

        SceneData* sceneData = ctx->GetData<SceneData>();

        DAVA::ScopedPtr<DAVA::Entity> newNode(new DAVA::Entity());
        newNode->SetName("dummyEntity");
        newNode->AddComponent(new DAVA::WASDControllerComponent());
        sceneData->GetScene()->AddNode(newNode.get());

        auto step1 = [this]() {
            MatchEntitiesWithScene();
            InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);
        };

        auto step2 = [this, ctx]() {
            MatchEntitiesWithScene();
            GetContextManager()->ActivateContext(ctx->GetID());
        };

        auto finalStep = [this]() {
            MatchEntitiesWithScene();
            InvokeOperation(REGlobal::CloseAllScenesOperation.ID, false);
        };

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(step1))
        .WillOnce(Invoke(step2))
        .WillOnce(Invoke(finalStep))
        .WillOnce(Return());
    }

    DAVA_TEST (ChangeSceneInplaceScenesTest)
    {
        TEST_VERIFY(sceneTreeView != nullptr);

        using namespace DAVA::TArc;
        using namespace ::testing;

        ContextAccessor* accessor = GetAccessor();
        TEST_VERIFY(accessor->GetActiveContext() == nullptr);
        InvokeOperation(REGlobal::CreateFirstSceneOperation.ID);

        DataContext* ctx = accessor->GetActiveContext();
        TEST_VERIFY(ctx != nullptr);

        SceneData* sceneData = ctx->GetData<SceneData>();
        SceneEditor2* scene = sceneData->GetScene().Get();

        DAVA::ScopedPtr<DAVA::Entity> newNode(new DAVA::Entity());
        newNode->SetName("dummyEntity");
        newNode->AddComponent(new DAVA::WASDControllerComponent());
        sceneData->GetScene()->AddNode(newNode.get());

        DAVA::Entity* entityForDelete = new DAVA::Entity();
        entityForDelete->SetName("nodeForDelete");

        auto step1 = [=]() {
            MatchEntitiesWithScene();

            scene->AddNode(entityForDelete);

            {
                DAVA::ScopedPtr<DAVA::Entity> subNode(new DAVA::Entity());
                subNode->SetName("subNode1");
                newNode->AddNode(subNode.get());
            }

            {
                DAVA::ScopedPtr<DAVA::Entity> subNode(new DAVA::Entity());
                subNode->SetName("subNode2");
                newNode->AddNode(subNode.get());
            }
        };

        auto step2 = [=]() {
            MatchEntitiesWithScene();

            scene->RemoveNode(entityForDelete);
            scene->GetSystem<SceneTreeSystem>()->Process(0.01f);
        };

        auto finalStep = [=]() {
            entityForDelete->Release();
            MatchEntitiesWithScene();
            InvokeOperation(REGlobal::CloseAllScenesOperation.ID, false);
        };

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(step1))
        .WillOnce(Invoke(step2))
        .WillOnce(Invoke(finalStep))
        .WillOnce(Return());
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ReflectionExtensionsModule)
    DECLARE_TESTED_MODULE(Mock::ProjectManagerModule)
    DECLARE_TESTED_MODULE(SceneManagerModule)
    DECLARE_TESTED_MODULE(SceneTreeModule)
    END_TESTED_MODULES()
};
