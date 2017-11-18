#include "EditorPhysics/EditorPhysicsModule.h"
#include "EditorPhysics/Private/EditorPhysicsSystem.h"
#include "EditorPhysics/Private/EditorIntegrationHelper.h"
#include "EditorPhysics/Private/EditorPhysicsData.h"
#include "EditorPhysics/Private/PhysicsWidget.h"

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Qt/QtString.h>

#include <Scene3D/Scene.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Render/Highlevel/RenderObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

#include <Physics/VehicleWheelComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleTankComponent.h>
#include <Physics/ConvexHullShapeComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/BoxShapeComponent.h>

#include <QAction>
#include <QList>

void EditorPhysicsModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace EditorPhysicsDetail;
    DAVA::Scene* scene = ExtractScene(context);
    DVASSERT(scene != nullptr);

    std::unique_ptr<EditorPhysicsData> data(new EditorPhysicsData());
    data->system = new EditorPhysicsSystem(scene);
    scene->AddSystem(data->system, 0, DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    context->CreateData(std::move(data));
}

void EditorPhysicsModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace EditorPhysicsDetail;
    DAVA::Scene* scene = ExtractScene(context);
    DVASSERT(scene != nullptr);

    EditorPhysicsData* data = context->GetData<EditorPhysicsData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->system != nullptr);
    scene->RemoveSystem(data->system);
    DAVA::SafeDelete(data->system);
    context->DeleteData<EditorPhysicsData>();
}

void EditorPhysicsModule::PostInit()
{
    using namespace DAVA::TArc;

    QWidget* physicsPanel = new PhysicsWidget(GetAccessor(), GetUI());

    UI* ui = GetUI();

    DockPanelInfo info;
    info.title = QString("Physics");
    info.area = Qt::LeftDockWidgetArea;

    PanelKey key("Physics", info);
    ui->AddView(DAVA::TArc::mainWindowKey, key, physicsPanel);

    QAction* actionAddCar = new QAction("Car", nullptr);
    connections.AddConnection(actionAddCar, &QAction::triggered, [this]() {
        CreateCarEntity();
    });
    ActionPlacementInfo placementInfoCar(CreateMenuPoint(QList<QString>() << "menuCreateNode"
                                                                          << "menuAdd",
                                                         InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "actionAddPath")));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfoCar, actionAddCar);

    QAction* actionAddTank = new QAction("Tank", nullptr);
    connections.AddConnection(actionAddTank, &QAction::triggered, [this]() {
        CreateTankEntity();
    });
    ActionPlacementInfo placementInfoTank(CreateMenuPoint(QList<QString>() << "menuCreateNode"
                                                                           << "menuAdd",
                                                          InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "Car")));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfoTank, actionAddTank);
}

DAVA::RenderObject* CreateVehicleWheelRenderObject(DAVA::float32 radius, DAVA::float32 width)
{
    using namespace DAVA;

    // Generate wheel mesh triangles

    const size_t segments = 36;

    Array<Vector3, segments * 2> vertices;
    for (size_t i = 0; i < segments; ++i)
    {
        const float32 angle = i * PI * 2.0f / segments;
        const float32 x = radius * std::cos(angle);
        const float32 z = radius * std::sin(angle);
        vertices[2 * i + 0] = Vector3(x, -width / 2.0f, z);
        vertices[2 * i + 1] = Vector3(x, +width / 2.0f, z);
    }

    // Sectors * 2 triangles
    Array<uint16, segments * 2 * 3> indices;
    for (uint16 i = 0; i < segments; ++i)
    {
        if (i == segments - 1)
        {
            // Loop with the first indices

            indices[i * 6 + 0] = i * 2 + 0;
            indices[i * 6 + 1] = 0;
            indices[i * 6 + 2] = i * 2 + 1;

            indices[i * 6 + 3] = i * 2 + 1;
            indices[i * 6 + 4] = 0;
            indices[i * 6 + 5] = 1;
        }
        else
        {
            indices[i * 6 + 0] = i * 2 + 0;
            indices[i * 6 + 1] = i * 2 + 2;
            indices[i * 6 + 2] = i * 2 + 1;

            indices[i * 6 + 3] = i * 2 + 1;
            indices[i * 6 + 4] = i * 2 + 2;
            indices[i * 6 + 5] = i * 2 + 3;
        }
    }

    // Create render object

    RefPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName("VehicleWheelMaterial"));
    material->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);
    material->AddProperty(FastName("color"), Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);

    RefPtr<PolygonGroup> group(new PolygonGroup());
    group->SetPrimitiveType(rhi::PRIMITIVE_TRIANGLELIST);
    group->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), segments * 2);
    memcpy(group->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(group->indexArray, indices.data(), indices.size() * sizeof(uint16));
    group->BuildBuffers();
    group->RecalcAABBox();

    RenderObject* renderObject = new RenderObject();
    RenderBatch* batch = new RenderBatch();
    batch->SetMaterial(material.Get());
    batch->SetPolygonGroup(group.Get());
    batch->vertexLayoutId = static_cast<uint32>(-1);
    renderObject->AddRenderBatch(batch);

    return renderObject;
}

DAVA::Entity* CreateVehicleWheelEntity(DAVA::String name, DAVA::float32 radius, DAVA::float32 width, DAVA::float32 mass, DAVA::Vector3 localTranslation, DAVA::float32 maxHandbrakeTorque, DAVA::float32 maxSteerAngle)
{
    using namespace DAVA;

    Entity* wheel = new Entity();
    wheel->SetName(name.c_str());

    VehicleWheelComponent* wheelComponent = new VehicleWheelComponent();
    wheelComponent->SetRadius(radius);
    wheelComponent->SetWidth(width);
    wheelComponent->SetMaxHandbrakeTorque(maxHandbrakeTorque);
    wheelComponent->SetMaxSteerAngle(maxSteerAngle);
    wheel->AddComponent(wheelComponent);

    ConvexHullShapeComponent* shape = new ConvexHullShapeComponent();
    shape->SetOverrideMass(true);
    shape->SetMass(mass);
    wheel->AddComponent(shape);

    RenderComponent* wheel1Rendercomponent = new RenderComponent(CreateVehicleWheelRenderObject(radius, width));
    wheel->AddComponent(wheel1Rendercomponent);

    Matrix4 localTransform;
    localTransform.SetTranslationVector(localTranslation);
    wheel->SetLocalTransform(localTransform);

    return wheel;
}

void EditorPhysicsModule::CreateCarEntity()
{
    using namespace DAVA;

    Scene* scene = EditorPhysicsDetail::ExtractScene(GetAccessor()->GetActiveContext());

    const Vector3 chassisHalfDimensions = Vector3(2.5f, 1.0f, 1.25f);
    const float32 chassisMass = 1500.0f;
    const float32 wheelRadius = 0.35f;
    const float32 wheelWidth = 0.4f;
    const float32 wheelMass = 20.0f;

    // Root entity
    Entity* vehicleEntity = new DAVA::Entity();
    vehicleEntity->SetName("Vehicle (car)");
    VehicleComponent* vehicleComponent = new VehicleCarComponent();
    vehicleEntity->AddComponent(vehicleComponent);
    DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
    vehicleEntity->AddComponent(dynamicBody);

    // Wheel (front left)
    Entity* vehicleWheel4Entity = CreateVehicleWheelEntity("Wheel (front left)", wheelRadius, wheelWidth, wheelMass, Vector3(chassisHalfDimensions.x * 0.7f, chassisHalfDimensions.y * 0.7f, -chassisHalfDimensions.z), 0.0f, PI * 0.333f);
    vehicleEntity->AddNode(vehicleWheel4Entity);

    // Wheel (front right)
    Entity* vehicleWheel3Entity = CreateVehicleWheelEntity("Wheel (front right)", wheelRadius, wheelWidth, wheelMass, Vector3(chassisHalfDimensions.x * 0.7f, -chassisHalfDimensions.y * 0.7f, -chassisHalfDimensions.z), 0.0f, PI * 0.333f);
    vehicleEntity->AddNode(vehicleWheel3Entity);

    // Wheel (rear left)
    Entity* vehicleWheel2Entity = CreateVehicleWheelEntity("Wheel (rear left)", wheelRadius, wheelWidth, wheelMass, Vector3(-chassisHalfDimensions.x * 0.7f, chassisHalfDimensions.y * 0.7f, -chassisHalfDimensions.z), 4000.0f, 0.0f);
    vehicleEntity->AddNode(vehicleWheel2Entity);

    // Wheel (rear right)
    Entity* vehicleWheel1Entity = CreateVehicleWheelEntity("Wheel (rear right)", wheelRadius, wheelWidth, wheelMass, Vector3(-chassisHalfDimensions.x * 0.7f, -chassisHalfDimensions.y * 0.7f, -chassisHalfDimensions.z), 4000.0f, 0.0f);
    vehicleEntity->AddNode(vehicleWheel1Entity);

    // Chassis
    ScopedPtr<Entity> vehicleChassisEntity(new Entity());
    vehicleChassisEntity->SetName("Chassis");
    VehicleChassisComponent* chassisComponent = new VehicleChassisComponent();
    vehicleChassisEntity->AddComponent(chassisComponent);
    BoxShapeComponent* chassisShape = new BoxShapeComponent();
    chassisShape->SetHalfSize(chassisHalfDimensions);
    chassisShape->SetOverrideMass(true);
    chassisShape->SetMass(chassisMass);
    vehicleChassisEntity->AddComponent(chassisShape);
    vehicleEntity->AddNode(vehicleChassisEntity);

    scene->AddNode(vehicleEntity);
}

void EditorPhysicsModule::CreateTankEntity()
{
    using namespace DAVA;

    Scene* scene = EditorPhysicsDetail::ExtractScene(GetAccessor()->GetActiveContext());

    const Vector3 chassisHalfDimensions = Vector3(2.5f, 1.0f, 1.25f);
    const float32 chassisMass = 1500.0f;
    const float32 wheelRadius = 0.35f;
    const float32 wheelWidth = 0.4f;
    const float32 wheelMass = 20.0f;

    // Root entity
    Entity* vehicleEntity = new DAVA::Entity();
    vehicleEntity->SetName("Vehicle");
    VehicleComponent* vehicleComponent = new VehicleTankComponent();
    vehicleEntity->AddComponent(vehicleComponent);
    DynamicBodyComponent* dynamicBody = new DynamicBodyComponent();
    vehicleEntity->AddComponent(dynamicBody);

    // Wheels

    float mainwheelsz = -1.3f * chassisHalfDimensions.z;

    const uint32 wheelsNumber = 6;
    const float totalWheelsWidth = wheelWidth * wheelsNumber;

    float xOffset = -chassisHalfDimensions.x + (chassisHalfDimensions.x * 2.0f - wheelRadius * 2.0f);
    for (int i = 0; i < 6; ++i)
    {
        Entity* left = CreateVehicleWheelEntity("Wheel (left)", wheelRadius, wheelWidth, wheelMass, Vector3(xOffset, chassisHalfDimensions.y * 0.7f, mainwheelsz), 0.0f, 0.0f);
        vehicleEntity->AddNode(left);

        Entity* right = CreateVehicleWheelEntity("Wheel (right)", wheelRadius, wheelWidth, wheelMass, Vector3(xOffset, -chassisHalfDimensions.y * 0.7f, mainwheelsz), 0.0f, 0.0f);
        vehicleEntity->AddNode(right);

        xOffset -= wheelRadius * 2.0f;
    }

    // Chassis
    Entity* vehicleChassisEntity = new Entity();
    vehicleChassisEntity->SetName("Chassis");
    VehicleChassisComponent* chassisComponent = new VehicleChassisComponent();
    vehicleChassisEntity->AddComponent(chassisComponent);
    BoxShapeComponent* chassisShape = new BoxShapeComponent();
    chassisShape->SetHalfSize(chassisHalfDimensions);
    chassisShape->SetOverrideMass(true);
    chassisShape->SetMass(chassisMass);
    vehicleChassisEntity->AddComponent(chassisShape);
    vehicleEntity->AddNode(vehicleChassisEntity);

    scene->AddNode(vehicleEntity);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EditorPhysicsModule)
{
    DAVA::ReflectionRegistrator<EditorPhysicsModule>::Begin()
    .ConstructorByPointer()
    .End();
}
