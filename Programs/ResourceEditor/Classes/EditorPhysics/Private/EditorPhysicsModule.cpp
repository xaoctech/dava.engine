#include "Classes/EditorPhysics/EditorPhysicsModule.h"
#include "Classes/EditorPhysics/Private/EditorPhysicsSystem.h"
#include "Classes/EditorPhysics/Private/EditorPhysicsData.h"
#include "Classes/EditorPhysics/Private/PhysicsWidget.h"

#include <REPlatform/Commands/EntityAddCommand.h>
#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Global/PropertyPanelInterface.h>
#include <REPlatform/Global/SceneTree/CreateEntitySupport.h>
#include <REPlatform/Scene/SceneEditor2.h>

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Utils/ReflectedPairsVector.h>
#include <TArc/Qt/QtString.h>
#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>
#include <TArc/Controls/ComboBox.h>

#include <Physics/PhysicsModule.h>
#include <Physics/Vehicles/VehicleWheelComponent.h>
#include <Physics/Vehicles/VehicleChassisComponent.h>
#include <Physics/Vehicles/VehicleCarComponent.h>
#include <Physics/Vehicles/VehicleTankComponent.h>
#include <Physics/Core/ConvexHullShapeComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/BoxShapeComponent.h>

#include <Scene3D/Scene.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Render/Highlevel/RenderObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Base/Type.h>

#include <QAction>
#include <QList>
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

namespace EditorPhysicsDetail
{
using namespace DAVA;

class PhysicsMaterialComponentValue : public BaseComponentValue
{
public:
    PhysicsMaterialComponentValue()
    {
        PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
        DVASSERT(module != nullptr);

        Vector<FastName> materialNames = module->GetMaterialNames();

        std::sort(materialNames.begin(), materialNames.end(), [](const FastName& name1, const FastName& name2) {
            DVASSERT(name1.IsValid() == true);
            DVASSERT(name2.IsValid() == true);
            return strcmp(name1.c_str(), name2.c_str()) < 0;
        });

        materials.values.emplace_back(FastName(), String("Default material"));
        for (const FastName& name : materialNames)
        {
            materials.values.emplace_back(name, name.c_str());
        }
    }

    Any GetMultipleValue() const override
    {
        return Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        if (currentValue.IsEmpty())
        {
            return true;
        }
        return newValue != currentValue;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        ComboBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ComboBox::Fields::Enumerator] = "materials";
        params.fields[ComboBox::Fields::Value] = "currentMaterial";
        params.fields[ComboBox::Fields::MultipleValueText] = "unregisteredMaterial";
        return new ComboBox(params, wrappersProcessor, model, parent);
    }

private:
    Any GetCurrentMaterial()
    {
        return GetValue();
    }

    void SetCurrentMaterial(const Any& materialName)
    {
        SetValue(materialName);
    }

    ReflectedPairsVector<FastName, String> materials;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(PhysicsMaterialComponentValue, BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<PhysicsMaterialComponentValue>::Begin()
        .Field("materials", &PhysicsMaterialComponentValue::materials)
        .Field("currentMaterial", &PhysicsMaterialComponentValue::GetCurrentMaterial, &PhysicsMaterialComponentValue::SetCurrentMaterial)
        .Field("unregisteredMaterial", [](PhysicsMaterialComponentValue*) -> String { return "Unregistered material"; }, nullptr)
        .End();
    }
};

class PhysicsEditorCreator : public EditorComponentExtension
{
public:
    std::unique_ptr<DAVA::BaseComponentValue> GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const override
    {
        std::shared_ptr<DAVA::PropertyNode> parentNode = node->parent.lock();
        if (parentNode != nullptr)
        {
            const DAVA::Type* valueType = node->cachedValue.GetType();
            if (node->propertyType == DAVA::PropertyNode::RealProperty &&
                valueType == DAVA::Type::Instance<DAVA::FastName>() &&
                parentNode->cachedValue.CanCast<DAVA::CollisionShapeComponent*>() &&
                node->field.key.Cast<DAVA::String>() == "material")
            {
                return std::make_unique<PhysicsMaterialComponentValue>();
            }
        }

        return EditorComponentExtension::GetEditor(node);
    }
};

class EditorPhysicsGlobalData : public DAVA::TArcDataNode
{
public:
    std::shared_ptr<DAVA::EditorComponentExtension> physicsExtension;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EditorPhysicsGlobalData, DAVA::TArcDataNode)
    {
        ReflectionRegistrator<EditorPhysicsGlobalData>::Begin()
        .End();
    }
};
} // namespace EditorPhysicsDetail

EditorPhysicsModule::EditorPhysicsModule()
{
}

void EditorPhysicsModule::OnContextCreated(DAVA::DataContext* context)
{
    DAVA::SceneEditor2* scene = context->GetData<DAVA::SceneData>()->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<EditorPhysicsData> data(new EditorPhysicsData());
    data->system = new EditorPhysicsSystem(scene);
    scene->AddSystem(data->system);

    context->CreateData(std::move(data));
}

void EditorPhysicsModule::OnContextDeleted(DAVA::DataContext* context)
{
    DAVA::SceneEditor2* scene = context->GetData<DAVA::SceneData>()->GetScene().Get();
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
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    binder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor descr;
        descr.type = ReflectedTypeDB::Get<ProjectManagerData>();
        descr.fieldName = FastName(ProjectManagerData::ProjectPathProperty);
        binder->BindField(descr, [](const Any& v) {
            PhysicsModule* module = DAVA::GetEngineContext()->moduleManager->GetModule<DAVA::PhysicsModule>();
            module->ReleaseMaterials();
        });
    }

    EditorPhysicsDetail::EditorPhysicsGlobalData* globalData = new EditorPhysicsDetail::EditorPhysicsGlobalData();
    globalData->physicsExtension.reset(new EditorPhysicsDetail::PhysicsEditorCreator());
    accessor->GetGlobalContext()->CreateData(std::unique_ptr<DAVA::TArcDataNode>(globalData));

    QWidget* physicsPanel = new PhysicsWidget(GetAccessor(), GetUI());

    UI* ui = GetUI();

    DockPanelInfo info;
    info.title = QString("Physics");
    info.area = Qt::LeftDockWidgetArea;

    PanelKey key("Physics", info);
    ui->AddView(DAVA::mainWindowKey, key, physicsPanel);
}

void EditorPhysicsModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<DAVA::PropertyPanelInterface>())
    {
        DAVA::PropertyPanelInterface* propertyPanel = QueryInterface<DAVA::PropertyPanelInterface>();
        EditorPhysicsDetail::EditorPhysicsGlobalData* data = GetAccessor()->GetGlobalContext()->GetData<EditorPhysicsDetail::EditorPhysicsGlobalData>();
        propertyPanel->RegisterExtension(data->physicsExtension);
    }
}

void EditorPhysicsModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<DAVA::PropertyPanelInterface>())
    {
        DAVA::PropertyPanelInterface* propertyPanel = QueryInterface<DAVA::PropertyPanelInterface>();
        EditorPhysicsDetail::EditorPhysicsGlobalData* data = GetAccessor()->GetGlobalContext()->GetData<EditorPhysicsDetail::EditorPhysicsGlobalData>();
        propertyPanel->UnregisterExtension(data->physicsExtension);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(EditorPhysicsModule)
{
    DAVA::ReflectionRegistrator<EditorPhysicsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(EditorPhysicsModule);
