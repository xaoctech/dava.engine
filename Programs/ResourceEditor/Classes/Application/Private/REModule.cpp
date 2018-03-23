#include "Classes/Application/REModule.h"
#include "Classes/Application/FileSystemData.h"
#include "Classes/Application/Private/SettingsConverter.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"

#include <REPlatform/DataNodes/Settings/RESettings.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/EditorStatisticsSystem.h>
#include <REPlatform/Global/MenuScheme.h>

#include <TArc/WindowSubSystem/Private/UIManager.h>
#include <TArc/DataProcessing/TArcDataNode.h>

#include <Base/ObjectFactory.h>
#include <Entity/ComponentManager.h>
#include <Engine/EngineContext.h>
#include <FileSystem/LocalizationSystem.h>
#include <UI/UIControlSystem.h>
#include <UI/Render/UIRenderSystem.h>

#include <QPointer>

namespace REModuleDetail
{
class REGlobalData : public DAVA::TArcDataNode
{
public:
    REGlobalData(DAVA::UI* ui)
    {
        textureCache = new TextureCache();
        mainWindow = new QtMainWindow(ui);
    }

    ~REGlobalData()
    {
        DAVA::SafeRelease(textureCache);

        if (!mainWindow.isNull())
        {
            QtMainWindow* mainWindowPointer = mainWindow.data();
            mainWindow.clear();
            DAVA::SafeDelete(mainWindowPointer);
        }
    }

    TextureCache* textureCache = nullptr;
    QPointer<QtMainWindow> mainWindow;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(REGlobalData)
    {
    };
};
}

REModule::REModule()
{
    using namespace DAVA;

    DAVA::ObjectFactory::Instance()->RegisterObjectCreator("DAVA::QueryObjectDataComponent", &DAVA::CreateQueryDataComponent, typeid(QueryObjectDataComponent), sizeof(QueryObjectDataComponent));

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderStatsSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(QueryObjectDataComponent);

    GetEngineContext()->componentManager->RegisterComponent<QueryObjectDataComponent>();
}

REModule::~REModule()
{
    GetAccessor()->GetGlobalContext()->DeleteData<REModuleDetail::REGlobalData>();
}

void REModule::PostInit()
{
    DAVA::ContextAccessor* accessor = GetAccessor();
    ConvertSettingsIfNeeded(accessor->GetPropertiesHolder(), accessor);

    const DAVA::EngineContext* engineContext = accessor->GetEngineContext();
    engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext->localizationSystem->SetCurrentLocale("en");
    engineContext->uiControlSystem->GetRenderSystem()->SetClearColor(DAVA::Color(0.25f, 0.275f, 0.3f, 1.0f));

    using TData = REModuleDetail::REGlobalData;
    DAVA::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<TData>(GetUI()));
    TData* globalData = globalContext->GetData<TData>();

    DAVA::UI* ui = GetUI();
    DAVA::BuildMenuPrositionsFromLegacy(globalData->mainWindow->menuBar());
    ui->InjectWindow(DAVA::mainWindowKey, globalData->mainWindow);
    globalData->mainWindow->AfterInjectInit();
    globalData->mainWindow->EnableGlobalTimeout(true);

    RegisterOperation(DAVA::ShowMaterial.ID, this, &REModule::ShowMaterial);

    globalContext->CreateData(std::make_unique<FileSystemData>());
}

void REModule::ShowMaterial(DAVA::NMaterial* material)
{
    using TData = REModuleDetail::REGlobalData;
    DAVA::DataContext* globalContext = GetAccessor()->GetGlobalContext();
    TData* globalData = globalContext->GetData<TData>();
    globalData->mainWindow->OnMaterialEditor(material);
}
