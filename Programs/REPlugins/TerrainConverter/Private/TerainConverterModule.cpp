#include "TerainConverterModule.h"

//#include <Classes/SceneManager/SceneData.h>

#include <TArc/PluginsManager/TArcPlugin.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/Common.h>

#include <Reflection/ReflectedTypeDB.h>
#include <Base/FastName.h>
#include <Base/BaseTypes.h>

void TerrainConverterGUIModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    QtAction* action = new QtAction(accessor, "Convert Map");
    //FieldDescriptor descr;
    //descr.type = ReflectedTypeDB::Get<SceneData>();
    //descr.fieldName = FastName(SceneData::scenePropertyName);
    //action->SetStateUpdationFunction(QtAction::Enabled, descr,[](const Any& v) {
    //    return v.IsEmpty() == false;
    //});

    QUrl menuPoint = CreateMenuPoint(QList<QString>() << "Terrain Converter");
    ActionPlacementInfo info(menuPoint);
    ui->AddAction(DAVA::mainWindowKey, info, action);
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
