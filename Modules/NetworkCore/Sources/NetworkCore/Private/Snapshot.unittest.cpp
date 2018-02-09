#include "NetworkCore/Snapshot.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Systems/NetworkIdSystem.h"
#include "NetworkCore/Scene3D/Systems/SnapshotSystemServer.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <UnitTests/UnitTests.h>

#include <random>

using namespace DAVA;

namespace SnapshotTestDetails
{
struct TestData
{
    float32 f = 1.0f;
    uint32 u = 2;
    Vector3 v3 = { 1.0f, 2.0f, 3.0f };
    Matrix4 m4;
    Array<float32, 3> arr = { 3.0f, 2.0f, 1.0f };

    TestData() = default;
    TestData(TestData&&) = default;
    TestData(const TestData&) = default;

    bool operator==(const TestData& data)
    {
        return (f == data.f && u == data.u && v3 == data.v3 && m4 == data.m4 && arr == data.arr);
    }

    bool operator!=(const TestData& data)
    {
        return !this->operator==(data);
    }
};

struct TestComponent : public Component
{
    float32 f = 1.0f;
    uint32 u = 2;
    Vector3 v3 = { 1.0f, 2.0f, 3.0f };
    Matrix4 m4;
    Array<float32, 3> arr = { 3.0f, 2.0f, 1.0f };

    Component* Clone(Entity* toEntity) override
    {
        TestComponent* rc = new TestComponent();
        rc->SetEntity(toEntity);
        rc->f = f;
        rc->u = u;
        rc->v3 = v3;
        rc->m4 = m4;
        return rc;
    }

    TestData GetData()
    {
        TestData data;
        data.f = f;
        data.u = u;
        data.v3 = v3;
        data.m4 = m4;
        data.arr = arr;
        return data;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestComponent, Component)
    {
        ReflectionRegistrator<TestComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
        .ConstructorByPointer<>()
        .Field("f", &TestComponent::f)[M::Replicable()]
        .Field("u", &TestComponent::u)[M::Replicable()]
        .Field("v3", &TestComponent::v3)[M::Replicable()]
        .Field("m4", &TestComponent::m4)[M::Replicable(M::Privacy::PRIVATE)]
        .Field("arr", &TestComponent::arr)[M::Replicable()]
        .End();
    }
};

struct CustomComponent : public Component
{
    Component* Clone(Entity* toEntity) override
    {
        CustomComponent* rc = new CustomComponent();
        rc->SetEntity(toEntity);
        return rc;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CustomComponent, Component)
    {
        ReflectionRegistrator<CustomComponent>::Begin()
        .ConstructorByPointer<>()
        .End();
    }
};
}

DAVA_TESTCLASS (SnapshotTest)
{
    Scene* testScene = nullptr;
    Snapshot* snapshot = nullptr;
    NetworkTimeSingleComponent* timeSingleComponent = nullptr;

    const float32 fixedUpdateTime = 16.0f;

    SnapshotTest()
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotTestDetails::TestComponent);
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapshotTestDetails::CustomComponent);

        GetEngineContext()->componentManager->RegisterComponent(Type::Instance<SnapshotTestDetails::TestComponent>());
        GetEngineContext()->componentManager->RegisterComponent(Type::Instance<SnapshotTestDetails::CustomComponent>());

        testScene = new Scene();
        testScene->AddSystem(new NetworkIdSystem(testScene));

        timeSingleComponent = testScene->GetSingletonComponent<NetworkTimeSingleComponent>();

        SnapshotSystemServer* ss = new SnapshotSystemServer(testScene);
        snapshot = &ss->snapshot;

        testScene->AddSystem(ss);
    }

    ~SnapshotTest()
    {
        SafeRelease(testScene);
    }

    Entity* CreateEntity()
    {
        static NetworkID staticId(1);

        Entity* entity = new Entity();
        entity->SetName(Format("entity-%u", static_cast<uint32>(staticId)).c_str());

        NetworkReplicationComponent* nrc = new NetworkReplicationComponent();
        nrc->SetNetworkID(staticId++);
        entity->AddComponent(nrc);

        SnapshotTestDetails::TestComponent* tc = new SnapshotTestDetails::TestComponent();
        entity->AddComponent(tc);

        SnapshotTestDetails::CustomComponent* cc = new SnapshotTestDetails::CustomComponent();
        entity->AddComponent(cc);

        return entity;
    }

    bool TestEntity(Entity * entity)
    {
        static uint32 cid = GetEngineContext()->componentManager->GetRuntimeComponentIndex(Type::Instance<SnapshotTestDetails::TestComponent>());

        NetworkID networkID = NetworkCoreUtils::GetEntityId(entity);

        TEST_VERIFY(entity == snapshot->entities[networkID].entity);

        SnapshotComponent* sc = &snapshot->entities[networkID].components[SnapshotComponentKey(cid, 0)];
        SnapshotTestDetails::TestComponent* tc = entity->GetComponent<SnapshotTestDetails::TestComponent>();

        TEST_VERIFY(sc->fields[0].value.Get<float32>() == tc->f);
        TEST_VERIFY(sc->fields[1].value.Get<uint32>() == tc->u);
        TEST_VERIFY(sc->fields[2].value.Get<Vector3>() == tc->v3);
        TEST_VERIFY(sc->fields[3].value.Get<Matrix4>() == tc->m4);

        return true;
    }

    void RandomizeTestComponent(SnapshotTestDetails::TestComponent * tc)
    {
        static std::random_device rd;
        static std::default_random_engine re(rd());
        static std::uniform_int_distribution<uint32> idist(0, 1000);
        static std::uniform_real_distribution<float32> fdist(-10.0f, 10.0f);

        tc->f = fdist(re);
        tc->u = idist(re);
        tc->v3 = Vector3(fdist(re), fdist(re), fdist(re));
        tc->m4 = Matrix4(fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re), fdist(re));

        for (size_t i = 0; i < tc->arr.size(); ++i)
        {
            tc->arr[i] = fdist(re);
        }
    }

    DAVA_TEST (SnapshotUtils)
    {
        Entity* e1 = CreateEntity();
        Entity* e2 = new Entity();

        TEST_VERIFY(NetworkID::SCENE_ID == NetworkCoreUtils::GetEntityId(nullptr));
        TEST_VERIFY(NetworkID::SCENE_ID == NetworkCoreUtils::GetEntityId(testScene));
        TEST_VERIFY(NetworkID::INVALID != NetworkCoreUtils::GetEntityId(e1));
        TEST_VERIFY(NetworkID::INVALID == NetworkCoreUtils::GetEntityId(e2));

        e1->Release();
        e2->Release();
    }

    DAVA_TEST (AddRemoveEntity)
    {
        Entity* e1 = CreateEntity();

        testScene->AddNode(e1);

        Snapshot s1 = *snapshot;

        auto x = e1->GetComponent<SnapshotTestDetails::TestComponent>();
        if (x)
        {
            RandomizeTestComponent(x);
        }
        testScene->Update(fixedUpdateTime);
        TEST_VERIFY(TestEntity(e1));

        uint8 buf[1400];
        NetworkID nid = NetworkCoreUtils::GetEntityId(e1);
        size_t n = SnapshotUtils::CreateSnapshotDiff(&s1, snapshot, nid, Metas::Privacy::PRIVATE, buf, 1400);

        size_t j = SnapshotUtils::GetSnapshotDiffSize(buf, n);

        Snapshot t; // = s1;
        size_t m = SnapshotUtils::ApplySnapshotDiff(&s1, &t, nid, buf, n, [](const SnapshotApplyParam& param) {});

        Entity* e2 = CreateEntity();
        Entity* e3 = CreateEntity();
        e2->AddNode(e3);
        e3->Release();
        testScene->AddNode(e2);

        testScene->Update(fixedUpdateTime);
        TEST_VERIFY(TestEntity(e2));
        TEST_VERIFY(TestEntity(e3));

        testScene->RemoveNode(e2);
        e2->Release();

        testScene->Update(fixedUpdateTime);
        TEST_VERIFY(TestEntity(e1));

        testScene->RemoveNode(e1);
        e1->Release();
    }

    DAVA_TEST (ChangeValue)
    {
        Entity* e1 = CreateEntity();
        Entity* e2 = CreateEntity();
        Entity* e3 = CreateEntity();

        e2->AddNode(e3);
        e3->Release();
        testScene->AddNode(e1);
        testScene->AddNode(e2);

        // every component changed
        {
            RandomizeTestComponent(e1->GetComponent<SnapshotTestDetails::TestComponent>());
            RandomizeTestComponent(e2->GetComponent<SnapshotTestDetails::TestComponent>());
            RandomizeTestComponent(e3->GetComponent<SnapshotTestDetails::TestComponent>());

            testScene->Update(fixedUpdateTime);
            TEST_VERIFY(TestEntity(e1));
            TEST_VERIFY(TestEntity(e2));
            TEST_VERIFY(TestEntity(e3));
        }

        // one component changed
        {
            RandomizeTestComponent(e1->GetComponent<SnapshotTestDetails::TestComponent>());

            testScene->Update(fixedUpdateTime);
            TEST_VERIFY(TestEntity(e1));
            TEST_VERIFY(TestEntity(e2));
            TEST_VERIFY(TestEntity(e3));
        }

        // no component changes
        {
            testScene->Update(fixedUpdateTime);
            TEST_VERIFY(TestEntity(e1));
            TEST_VERIFY(TestEntity(e2));
            TEST_VERIFY(TestEntity(e3));
        }

        testScene->RemoveNode(e1);
        e1->Release();
    }

    bool CheckSSCSnapshot(bool has, uint32 frameId)
    {
        SnapshotSingleComponent* ssc = testScene->GetSingletonComponent<SnapshotSingleComponent>();
        Snapshot* s = ssc->GetServerSnapshot(frameId);

        if (has)
        {
            TEST_VERIFY(nullptr != s);

            if (nullptr != s)
            {
                TEST_VERIFY(s->frameId == frameId);
                return (s->frameId == frameId);
            }
        }
        else
        {
            TEST_VERIFY(nullptr == s);
            return (nullptr == s);
        }

        return false;
    };

    DAVA_TEST (SingleComponentCreateGet)
    {
        SnapshotSingleComponent* ssc = testScene->GetSingletonComponent<SnapshotSingleComponent>();

        TEST_VERIFY(CheckSSCSnapshot(false, 3));

        TEST_VERIFY(nullptr != ssc->CreateServerSnapshot(3));
        TEST_VERIFY(CheckSSCSnapshot(true, 3));
        TEST_VERIFY(CheckSSCSnapshot(false, 2));
        TEST_VERIFY(CheckSSCSnapshot(false, 1));
        TEST_VERIFY(CheckSSCSnapshot(false, 4));

        size_t historySize = ssc->serverHistory.size();
        size_t overflowFrameId = historySize * 2;
        TEST_VERIFY(nullptr != ssc->CreateServerSnapshot(overflowFrameId));
        TEST_VERIFY(nullptr != ssc->CreateServerSnapshot(overflowFrameId - historySize + 1));

        TEST_VERIFY(CheckSSCSnapshot(true, overflowFrameId));
        TEST_VERIFY(CheckSSCSnapshot(true, overflowFrameId - historySize + 1));
        TEST_VERIFY(CheckSSCSnapshot(false, overflowFrameId - 1));
        TEST_VERIFY(CheckSSCSnapshot(false, 1));
        TEST_VERIFY(CheckSSCSnapshot(false, 2));

        TEST_VERIFY(nullptr == ssc->CreateServerSnapshot(1));

        timeSingleComponent->SetFrameId(overflowFrameId);
    }

    DAVA_TEST (SnapshotApply)
    {
        Entity* e1 = CreateEntity();
        testScene->AddNode(e1);

        SnapshotTestDetails::TestComponent* c1 = e1->GetComponent<SnapshotTestDetails::TestComponent>();

        NetworkID networkID = NetworkCoreUtils::GetEntityId(e1);
        uint32 componentId = ComponentUtils::GetRuntimeIndex(c1);

        // component values change
        {
            RandomizeTestComponent(c1);
            SnapshotTestDetails::TestData dataBeforeSnap = c1->GetData();

            testScene->Update(fixedUpdateTime);

            RandomizeTestComponent(c1);

            // apply snapshot and check values are ok
            SnapshotUtils::ApplySnapshot(snapshot, networkID, e1);
            SnapshotTestDetails::TestData dataAfterSnap = c1->GetData();
            TEST_VERIFY(dataBeforeSnap == dataAfterSnap);
            TEST_VERIFY(nullptr != e1->GetComponent<SnapshotTestDetails::CustomComponent>());
        }

// component add/remove
#if 0
        {
            Snapshot s = *snapshot;
            e1->DetachComponent(c1);

            // check that snapshot apply will add component
            SnapshotUtils::ApplySnapshot(&s, entityId, e1);
            TEST_VERIFY(nullptr != e1->GetComponent<SnapshotTestDetails::TestComponent>());
            TEST_VERIFY(nullptr != e1->GetComponent<SnapshotTestDetails::CustomComponent>());

            // remove component and make new snapshot
            e1->RemoveComponent<SnapshotTestDetails::TestComponent>();
            testScene->Update(fixedUpdateTime);

            // add component back
            s = *snapshot;
            e1->AddComponent(c1);

            // check that snapshot apply will remove component
            SnapshotUtils::ApplySnapshot(&s, entityId, e1);
            TEST_VERIFY(nullptr == e1->GetComponent<SnapshotTestDetails::TestComponent>());
            TEST_VERIFY(nullptr != e1->GetComponent<SnapshotTestDetails::CustomComponent>());
        }
#endif

        e1->Release();
    }
};
