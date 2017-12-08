#include "UnitTests/UnitTests.h"

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/RefPtr.h"
#include "Base/ScopedPtr.h"
#include "Engine/Engine.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/ScreenPositionComponent.h"
#include "Scene3D/Systems/ScreenPositionSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "UI/UI3DView.h"
#include "UI/UIControlSystem.h"

DAVA_TESTCLASS (ScreenPositionSystemTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(DavaFramework)
    DECLARE_COVERED_FILES("ScreenPositionComponent.cpp")
    DECLARE_COVERED_FILES("ScreenPositionSystem.cpp")
    END_FILES_COVERED_BY_TESTS();
    
    DAVA::RefPtr<DAVA::Scene> scene;
    DAVA::RefPtr<DAVA::Entity> e1;
    DAVA::RefPtr<DAVA::Entity> e2;

    const static DAVA::FastName ENTITY_ONE;
    const static DAVA::FastName ENTITY_TWO;

    DAVA::RefPtr<DAVA::Entity> CreateEntity(const DAVA::FastName& name, const DAVA::Vector3& position)
    {
        using namespace DAVA;
        RefPtr<Entity> entity(new Entity());
        entity->SetName(name);

        Matrix4 m;
        m.BuildTranslation(position);
        entity->SetLocalTransform(m);

        entity->AddComponent(new ScreenPositionComponent());

        return entity;
    }

    void SetUp(const DAVA::String& testName) override
    {
        using namespace DAVA;
        scene = new Scene();
        
        ScreenPositionSystem* sps = scene->GetSystem<ScreenPositionSystem>();
        if(sps)
        {
            sps->SetViewport(Rect(0, 0, 500, 500));
        }
        
        e1 = CreateEntity(ENTITY_ONE, Vector3(0.f, 0.f, 0.f));
        scene->AddNode(e1.Get());
        e2 = CreateEntity(ENTITY_TWO, Vector3(1.f, 1.f, 1.f));
        scene->AddNode(e2.Get());

        ScopedPtr<Camera> camera(new Camera());
        camera->SetupPerspective(70.f, 1.0, 0.5f, 2500.f);
        camera->SetLeft(Vector3(1, 0, 0));
        camera->SetUp(Vector3(0, 0, 1.f));
        camera->SetTarget(Vector3(0, 0, 0));
        camera->SetPosition(Vector3(0, -2, 0));

        ScopedPtr<Entity> cameraEntity(new Entity());
        cameraEntity->AddComponent(new CameraComponent(camera));
        scene->AddNode(cameraEntity);
        scene->SetCurrentCamera(camera);
    }

    void TearDown(const DAVA::String& testName) override
    {
        e1 = nullptr;
        e2 = nullptr;
        scene = nullptr;        
    }

    DAVA_TEST (WithoutEntities)
    {
        scene->RemoveAllChildren();
        scene->Update(0.f);
        TEST_VERIFY(scene->GetChildrenCount() == 0);
    }

    DAVA_TEST (PositionAndDepthTest)
    {
        using namespace DAVA;
        
        const struct {
            RefPtr<Entity> e;
            Vector3 data;
            float32 eps;
        } testData[] = {
            { e1, { 250.f, 250.f, 3.f }, 0.1f },
            { e2, { 369.f, 131.f, 5.f }, 0.1f }
        };

        scene->Update(0.f);
        
        for(const auto& test : testData)
        {
            ScreenPositionComponent* spc = static_cast<ScreenPositionComponent*>(test.e->GetComponent(Component::SCREEN_POSITION_COMPONENT));
            TEST_VERIFY(spc != nullptr);
            Vector3 posAndDepth = spc->GetScreenPositionAndDepth();
            TEST_VERIFY(FLOAT_EQUAL_EPS(posAndDepth.x, test.data.x, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(posAndDepth.y, test.data.y, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(posAndDepth.z, test.data.z, test.eps));
        }
    }
    
    DAVA_TEST (CameraParamsTest)
    {
        using namespace DAVA;
        
        const struct {
            Vector3 position;
            Vector3 direction;
            Rect viewport;
            float32 eps;
        } test {
            { 0.f, -2.f, 0.f },
            { 0.f, 1.f, 0.f },
            { 0.f, 0.f, 500.f, 500.f },
            EPSILON
        };
        
        scene->Update(0.f);
        
        for(const auto& e : { e1, e2 })
        {
            ScreenPositionComponent* spc = static_cast<ScreenPositionComponent*>(e->GetComponent(Component::SCREEN_POSITION_COMPONENT));
            TEST_VERIFY(spc != nullptr);
            const Vector3& cameraPos = spc->GetCameraPosition();
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraPos.x, test.position.x, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraPos.y, test.position.y, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraPos.z, test.position.z, test.eps));
            const Vector3& cameraDir = spc->GetCameraDirection();
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraDir.x, test.direction.x, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraDir.y, test.direction.y, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraDir.z, test.direction.z, test.eps));
            const Rect& cameraViewport = spc->GetCameraViewport();
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraViewport.x, test.viewport.x, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraViewport.y, test.viewport.y, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraViewport.dx, test.viewport.dx, test.eps));
            TEST_VERIFY(FLOAT_EQUAL_EPS(cameraViewport.dy, test.viewport.dy, test.eps));
        }
    }

    DAVA_TEST (CameraViewportTest)
    {
        using namespace DAVA;

        ScreenPositionSystem* sys = scene->GetSystem<ScreenPositionSystem>();
        sys->SetViewport(Rect());
        TEST_VERIFY(sys->GetViewport() == Rect());
        
        scene->Update(0.f);
        
        for(const auto& e : { e1, e2 })
        {
            ScreenPositionComponent* spc = static_cast<ScreenPositionComponent*>(e->GetComponent(Component::SCREEN_POSITION_COMPONENT));
            TEST_VERIFY(spc != nullptr);
            const Rect& cameraViewport = spc->GetCameraViewport();
            TEST_VERIFY(cameraViewport == Rect());
            Vector3 posAndDepth = spc->GetScreenPositionAndDepth();
            TEST_VERIFY(posAndDepth == Vector3::Zero);
        }
    }
};

const DAVA::FastName ScreenPositionSystemTest::ENTITY_ONE("Entity1");
const DAVA::FastName ScreenPositionSystemTest::ENTITY_TWO("Entity2");
