#include "TerainConverterModule.h"
#include "Converter.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Global/REFileOperationsIntarface.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/Common.h>
#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/PluginsManager/TArcPlugin.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Functional/Signal.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

namespace TerrainConverterGUIModuleDetail
{
class ConvertTerrainOperation : public DAVA::REFileOperation
{
public:
    QIcon GetIcon() const override
    {
        return QIcon();
    }

    QString GetName() const override
    {
        return QStringLiteral("Convert for WoT Blitz");
    }

    eOperationType GetType() const override
    {
        return REFileOperation::eOperationType::EXPORT;
    }

    QString GetTargetFileFilter() const override
    {
        return "Scene File (*.sc2)";
    }

    void Apply(const DAVA::FilePath& filePath) const override
    {
        convertScene.Emit(filePath);
    }

    mutable DAVA::Signal<const DAVA::FilePath&> convertScene;
};

class TerrainConverterSettings : public DAVA::SettingsNode
{
public:
    DAVA::FilePath templatesPath;
    DAVA::FilePath clientDataSource;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TerrainConverterSettings, DAVA::SettingsNode)
    {
        DAVA::ReflectionRegistrator<TerrainConverterSettings>::Begin()[DAVA::M::DisplayName("Terrain Converter")]
        .ConstructorByPointer()
        .Field("Templates Path", &TerrainConverterSettings::templatesPath)
        .Field("WoT Blitz DataSource", &TerrainConverterSettings::clientDataSource)
        .End();
    }
};
}

TerrainConverterGUIModule::TerrainConverterGUIModule()
{
    using namespace TerrainConverterGUIModuleDetail;
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TerrainConverterSettings);
}

void TerrainConverterGUIModule::PostInit()
{
    using namespace DAVA;
    using namespace TerrainConverterGUIModuleDetail;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    QtAction* action = new QtAction(accessor, "Convert Map");
    ConvertTerrainOperation* operation = new ConvertTerrainOperation();
    operation->convertScene.Connect(this, &TerrainConverterGUIModule::OnConvertScene);
    convertOperation.reset(operation);

    const Type* sceneDataType = Type::Instance<SceneData>();
    FieldDescriptor descr;
    descr.type = ReflectedTypeDB::GetByType(sceneDataType);
    descr.fieldName = FastName(SceneData::scenePropertyName);
    action->SetStateUpdationFunction(QtAction::Enabled, descr, [](const Any& v) {
        return v.IsEmpty() == false;
    });

    QUrl menuPoint = CreateMenuPoint(QList<QString>() << "Terrain Converter");
    ActionPlacementInfo info(menuPoint);
    ui->AddAction(DAVA::mainWindowKey, info, action);
}

void TerrainConverterGUIModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    using namespace TerrainConverterGUIModuleDetail;
    if (interfaceType == DAVA::Type::Instance<DAVA::REFileOperationsInterface>())
    {
        DAVA::REFileOperationsInterface* fileOperations = QueryInterface<DAVA::REFileOperationsInterface>();
        fileOperations->RegisterFileOperation(convertOperation);
    }
}

void TerrainConverterGUIModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    using namespace TerrainConverterGUIModuleDetail;
    if (interfaceType == DAVA::Type::Instance<DAVA::REFileOperationsInterface>())
    {
        DAVA::REFileOperationsInterface* fileOperations = QueryInterface<DAVA::REFileOperationsInterface>();
        fileOperations->UnregisterFileOperation(convertOperation);
    }
}

void TerrainConverterGUIModule::OnConvertScene(const DAVA::FilePath& scenePath)
{
    using namespace TerrainConverterGUIModuleDetail;
    TerrainConverterSettings* settings = GetAccessor()->GetGlobalContext()->GetData<TerrainConverterSettings>();
    Converter con(settings->templatesPath, settings->clientDataSource);
    con.Do(scenePath, DAVA::Vector<DAVA::String>());
}

DAVA_VIRTUAL_REFLECTION_IMPL(TerrainConverterGUIModule)
{
    DAVA::ReflectionRegistrator<TerrainConverterGUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

START_PLUGINS_DECLARATION();
DECLARE_PLUGIN(TerrainConverterGUIModule, DAVA::TArcPlugin::PluginDescriptor("ResourceEditor", "TerrainConverter", "Terrain Converter for Blitz", "Terrain Converter for Blitz", 0, 1));
END_PLUGINS_DECLARATION();
