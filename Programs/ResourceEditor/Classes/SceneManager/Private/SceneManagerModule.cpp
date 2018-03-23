#include "Classes/SceneManager/SceneManagerModule.h"
#include "Classes/SceneManager/Private/SceneRenderWidget.h"
#include "Classes/SceneManager/Private/SignalsAccumulator.h"
#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Scene/SceneSignals.h"
#include "Classes/Qt/TextureBrowser/TextureBrowser.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"
#include "Classes/Qt/Tools/ExportSceneDialog/ExportSceneDialog.h"

#include <REPlatform/Commands/Private/RECommandStack.h>
#include <REPlatform/Commands/TilemaskEditorCommands.h>
#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/Settings/GlobalSceneSettings.h>
#include <REPlatform/DataNodes/Settings/RESettings.h>
#include <REPlatform/DataNodes/SpritesPackerModule.h>
#include <REPlatform/Deprecated/EditorConfig.h>
#include <REPlatform/Global/Constants.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Global/MenuScheme.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/SceneHelper.h>
#include <REPlatform/Scene/Systems/CameraSystem.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/EditorParticlesSystem.h>
#include <REPlatform/Scene/Systems/EditorVegetationSystem.h>
#include <REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h>
#include <REPlatform/Scene/Systems/SelectionSystem.h>
#include <REPlatform/Scene/Systems/StructureSystem.h>
#include <REPlatform/Scene/Utils/SceneSaver.h>

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <QtTools/FileDialogs/FindFileDialog.h>
#include <QtTools/ProjectInformation/FileSystemCache.h>

#include <Asset/AssetManager.h>
#include <Base/Any.h>
#include <Base/FastName.h>
#include <Base/GlobalEnum.h>
#include <Base/String.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Functional/Function.h>
#include <Reflection/ReflectedObject.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/Reflection.h>
#include <Render/DynamicBufferAllocator.h>
#include <Render/Renderer.h>
#include <Render/TextureAssetLoader.h>
#include <Scene3D/Converters/SceneFileConverter.h>
#include <Scene3D/Level.h>
#include <Scene3D/Systems/StreamingSystem.h>

#include <QActionGroup>
#include <QList>
#include <QMimeData>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#define TEXTURE_GPU_FIELD_NAME "TexturesGPU"

namespace SceneManagerModuleDetail
{
class SceneManagerGlobalData : public DAVA::TArcDataNode
{
public:
    std::unique_ptr<SignalsAccumulator> signalsAccumulator;
    DAVA::eGPUFamily GetCurrentTexturesGPU() const
    {
        return DAVA::GetEngineContext()->assetManager->GetAssetLoader<DAVA::TextureAssetLoader>()->GetPrimaryGPUForLoading();
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SceneManagerGlobalData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<SceneManagerGlobalData>::Begin()
        .Field(TEXTURE_GPU_FIELD_NAME, &SceneManagerGlobalData::GetCurrentTexturesGPU, nullptr)
        .End();
    }
};

DAVA::Any SaveButtonTextFn(const DAVA::Any& value, bool saveAs)
{
    QString result = "Save";
    if (value.CanGet<DAVA::SceneData::eEditMode>() == true)
    {
        DAVA::SceneData::eEditMode mode = value.Get<DAVA::SceneData::eEditMode>();
        if (mode == DAVA::SceneData::Level)
        {
            result += " level";
        }
        else
        {
            result += " prefab";
        }
    }

    if (saveAs == true)
    {
        result += " as...";
    }

    return result;
};
}

//to use std::unique_ptr<FileSystemCache> sceneFilesCache with forward declaration
SceneManagerModule::SceneManagerModule()
{
    using namespace DAVA;
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SceneData);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GlobalSceneSettings);
}

SceneManagerModule::~SceneManagerModule() = default;

void SceneManagerModule::OnRenderSystemInitialized(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);

    DAVA::eGPUFamily family = GetAccessor()->GetGlobalContext()->GetData<DAVA::CommonInternalSettings>()->textureViewGPU;
    DAVA::GetEngineContext()->assetManager->GetAssetLoader<DAVA::TextureAssetLoader>()->SetGPULoadingOrder({ family });

    QtMainWindow* wnd = qobject_cast<QtMainWindow*>(GetUI()->GetWindow(DAVA::mainWindowKey));
    if (wnd != nullptr)
    {
        wnd->OnRenderingInitialized();
    }
}

bool SceneManagerModule::CanWindowBeClosedSilently(const DAVA::WindowKey& key, DAVA::String& requestWindowText)
{
    return false;
}

bool SceneManagerModule::ControlWindowClosing(const DAVA::WindowKey& key, QCloseEvent* event)
{
    using namespace DAVA;

    bool hasChanges = false;
    GetAccessor()->ForEachContext([&hasChanges](DataContext& context)
                                  {
                                      SceneData* data = context.GetData<SceneData>();
                                      DVASSERT(data != nullptr);
                                      DVASSERT(data->GetScene().Get() != nullptr);
                                      hasChanges |= data->GetScene()->IsChanged();
                                  });

    if (hasChanges == true)
    {
        ModalMessageParams params;
        params.title = QStringLiteral("Scene was changed");
        params.message = QStringLiteral("Do you want to quit anyway?");
        params.buttons = ModalMessageParams::Yes | ModalMessageParams::No;
        params.defaultButton = ModalMessageParams::No;
        if (GetUI()->ShowModalMessage(DAVA::mainWindowKey, params) == ModalMessageParams::Yes)
        {
            event->accept();
            CloseAllScenes(false);
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        CloseAllScenes(false);
    }

    return true;
}

bool SceneManagerModule::SaveOnWindowClose(const DAVA::WindowKey& key)
{
    return true;
}

void SceneManagerModule::RestoreOnWindowClose(const DAVA::WindowKey& key)
{
}

void SceneManagerModule::OnContextCreated(DAVA::DataContext* context)
{
    DAVA::SceneData* data = context->GetData<DAVA::SceneData>();
    SceneSignals::Instance()->EmitOpened(data->scene.Get());
}

void SceneManagerModule::OnContextDeleted(DAVA::DataContext* context)
{
    DAVA::SceneData* data = context->GetData<DAVA::SceneData>();
    SceneSignals::Instance()->EmitClosed(data->scene.Get());
}

void SceneManagerModule::OnContextWillBeChanged(DAVA::DataContext* current, DAVA::DataContext* newOne)
{
    using namespace DAVA;
    if (current == nullptr)
    {
        return;
    }

    SceneData* data = current->GetData<SceneData>();
    data->scene->Deactivate();
    SceneSignals::Instance()->EmitDeactivated(data->scene.Get());

    SceneManagerModuleDetail::SceneManagerGlobalData* globalData = current->GetData<SceneManagerModuleDetail::SceneManagerGlobalData>();
    globalData->signalsAccumulator.reset();
}

void SceneManagerModule::OnContextWasChanged(DAVA::DataContext* current, DAVA::DataContext* oldOne)
{
    using namespace DAVA;
    if (current == nullptr)
    {
        return;
    }

    SceneData* data = current->GetData<SceneData>();
    DVASSERT(data->scene.Get() != nullptr);
    SceneManagerModuleDetail::SceneManagerGlobalData* globalData = current->GetData<SceneManagerModuleDetail::SceneManagerGlobalData>();
    globalData->signalsAccumulator.reset(new SignalsAccumulator(data->scene.Get()));

    data->scene->Activate();
    SceneSignals::Instance()->EmitActivated(data->scene.Get());
}

void SceneManagerModule::OnWindowClosed(const DAVA::WindowKey& key)
{
}

void SceneManagerModule::PostInit()
{
    using namespace DAVA;

    QStringList extensions = { "sc2", "level", "prefab" };
    sceneFilesCache.reset(new FileSystemCache(extensions));

    ContextAccessor* accessor = GetAccessor();
    accessor->GetGlobalContext()->CreateData(std::make_unique<SceneManagerModuleDetail::SceneManagerGlobalData>());

    ProjectManagerData* projectData = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
    DVASSERT(projectData != nullptr);

    const DAVA::SpritesPackerModule* packer = projectData->GetSpritesModules();
    if (packer != nullptr)
    {
        connections.AddConnection(packer, &DAVA::SpritesPackerModule::SpritesReloaded, DAVA::MakeFunction(this, &SceneManagerModule::RestartParticles), Qt::QueuedConnection);
    }

    DAVA::UI* ui = GetUI();

    CreateModuleControls(ui);
    CreateSceneActions();
    CreateOtherModuleActions();

    RegisterOperations();

    fieldBinder.reset(new DAVA::FieldBinder(accessor));

    {
        DAVA::FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectManagerData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &SceneManagerModule::OnProjectPathChanged));
    }

    {
        DAVA::FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<GeneralSettings>();
        fieldDescr.fieldName = DAVA::FastName("recentScenesCount");
        fieldBinder->BindField(fieldDescr, [this](const DAVA::Any& v)
                               {
                                   recentItems->Truncate();
                               });
    }
}

void SceneManagerModule::CreateModuleControls(DAVA::UI* ui)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();

    DAVA::RenderWidget* engineRenderWidget = GetContextManager()->GetRenderWidget();
    renderWidget = new SceneRenderWidget(accessor, engineRenderWidget, this);

    QAction* deleteSelection = new QAction("Delete Selection", engineRenderWidget);
    {
        KeyBindableActionInfo info;
        info.blockName = "Edit";
        info.context = Qt::WidgetWithChildrenShortcut;
        info.defaultShortcuts << Qt::Key_Delete << Qt::CTRL + Qt::Key_Backspace;
        info.readOnly = true;
        MakeActionKeyBindable(deleteSelection, info);
    }
    engineRenderWidget->addAction(deleteSelection);
    connections.AddConnection(deleteSelection, &QAction::triggered, DAVA::MakeFunction(this, &SceneManagerModule::DeleteSelection));

    QAction* moveToSelection = new QAction("Move to selection", engineRenderWidget);
    {
        KeyBindableActionInfo info;
        info.blockName = "Edit";
        info.context = Qt::WindowShortcut;
        info.defaultShortcuts << QKeySequence(Qt::CTRL + Qt::Key_D);
        MakeActionKeyBindable(moveToSelection, info);
    }
    engineRenderWidget->addAction(moveToSelection);
    connections.AddConnection(moveToSelection, &QAction::triggered, DAVA::MakeFunction(this, &SceneManagerModule::MoveToSelection));

    PanelKey panelKey(QStringLiteral("SceneTabBar"), CentralPanelInfo());
    GetUI()->AddView(DAVA::mainWindowKey, panelKey, renderWidget);
}

void SceneManagerModule::CreateOtherModuleActions()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Undo/Redo
    DAVA::Function<DAVA::Any(DAVA::String, const DAVA::Any&)> makeUndoRedoText = [](DAVA::String prefix, const DAVA::Any& v) {
        DAVA::String descr = v.Cast<DAVA::String>("");
        if (descr.empty() == false)
        {
            return prefix + ": " + descr;
        }

        return prefix;
    };
    {
        QtAction* undo = new QtAction(accessor, QIcon(":/QtIcons/edit_undo.png"), QStringLiteral("Undo"));
        undo->SetStateUpdationFunction(QtAction::Enabled, MakeFieldDescriptor<SceneData>(SceneData::sceneCanUndoPropertyName), [](const DAVA::Any& v) {
            return v.Cast<bool>(false);
        });
        undo->SetStateUpdationFunction(QtAction::Text, MakeFieldDescriptor<SceneData>(SceneData::sceneUndoDescriptionPropertyName), Bind(makeUndoRedoText, "Undo", DAVA::_1));
        undo->SetStateUpdationFunction(QtAction::Tooltip, MakeFieldDescriptor<SceneData>(SceneData::sceneUndoDescriptionPropertyName), Bind(makeUndoRedoText, "Undo", DAVA::_1));

        KeyBindableActionInfo info;
        info.blockName = "Edit";
        info.context = Qt::ApplicationShortcut;
        info.defaultShortcuts.push_back(QKeySequence(Qt::CTRL + Qt::Key_Z));
        info.readOnly = true;
        MakeActionKeyBindable(undo, info);

        connections.AddConnection(undo, &QAction::triggered, [this]() {
            ContextAccessor* accessor = GetAccessor();
            DVASSERT(accessor->GetActiveContext() != nullptr);
            accessor->GetActiveContext()->GetData<SceneData>()->GetScene()->Undo();
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::AfterItem, "saveSeparator" }));
        ui->AddAction(mainWindowKey, placementInfo, undo);
    }

    {
        QtAction* redo = new QtAction(accessor, QIcon(":/QtIcons/edit_redo.png"), QStringLiteral("Redo"));
        redo->SetStateUpdationFunction(QtAction::Enabled, MakeFieldDescriptor<SceneData>(SceneData::sceneCanRedoPropertyName), [](const DAVA::Any& v) {
            return v.Cast<bool>(false);
        });
        redo->SetStateUpdationFunction(QtAction::Text, MakeFieldDescriptor<SceneData>(SceneData::sceneRedoDescriptionPropertyName), Bind(makeUndoRedoText, "Redo", DAVA::_1));
        redo->SetStateUpdationFunction(QtAction::Tooltip, MakeFieldDescriptor<SceneData>(SceneData::sceneRedoDescriptionPropertyName), Bind(makeUndoRedoText, "Redo", DAVA::_1));

        KeyBindableActionInfo info;
        info.blockName = "Edit";
        info.context = Qt::ApplicationShortcut;
        info.defaultShortcuts.push_back(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
        info.readOnly = true;
        MakeActionKeyBindable(redo, info);

        connections.AddConnection(redo, &QAction::triggered, [this]() {
            ContextAccessor* accessor = GetAccessor();
            DVASSERT(accessor->GetActiveContext() != nullptr);
            accessor->GetActiveContext()->GetData<SceneData>()->GetScene()->Redo();
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem, "Undo" }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint("mainToolBar", { InsertionParams::eInsertionMethod::AfterItem, "Undo" }));
        ui->AddAction(mainWindowKey, placementInfo, redo);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Menu View
    // GPU

    // View/GPU action
    QtAction* actionGPU = new QtAction(accessor, "GPU");

    FieldDescriptor descriptorGPU;
    descriptorGPU.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    descriptorGPU.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    actionGPU->SetStateUpdationFunction(QtAction::Enabled, descriptorGPU, [](const DAVA::Any& v) {
        return v.CanCast<SceneData::TSceneType>() && v.Cast<SceneData::TSceneType>().Get() != nullptr;
    });

    ActionPlacementInfo placementGPU(CreateMenuPoint(MenuItems::menuView, InsertionParams(InsertionParams::eInsertionMethod::BeforeItem)));
    ui->AddAction(DAVA::mainWindowKey, placementGPU, actionGPU);

    QActionGroup* actionGroup = new QActionGroup(actionGPU);
    actionGroup->setExclusive(true);

    DAVA::Vector<QAction*> gpuFormatActions;
    ActionPlacementInfo placement(CreateMenuPoint(QList<QString>() << MenuItems::menuView
                                                                   << "GPU"));

    auto createGpuAction = [&](DAVA::eGPUFamily gpu)
    {
        QtAction* action = new QtAction(accessor, GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu), nullptr);
        actionGroup->addAction(action);

        FieldDescriptor enabledFieldDescr;
        enabledFieldDescr.fieldName = DAVA::FastName(SceneData::sceneLandscapeToolsPropertyName);
        enabledFieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
        action->SetStateUpdationFunction(QtAction::Enabled, enabledFieldDescr, [](const DAVA::Any& v) -> DAVA::Any
                                         {
                                             return v.CanCast<DAVA::uint32>() && v.Cast<DAVA::uint32>() == 0;
                                         });

        FieldDescriptor checkedFieldDescr;
        checkedFieldDescr.fieldName = DAVA::FastName(TEXTURE_GPU_FIELD_NAME);
        checkedFieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneManagerModuleDetail::SceneManagerGlobalData>();
        action->SetStateUpdationFunction(QtAction::Checked, checkedFieldDescr, [gpu](const DAVA::Any& v)
                                         {
                                             return v.CanCast<DAVA::eGPUFamily>() && v.Cast<DAVA::eGPUFamily>() == gpu;
                                         });
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::ReloadAllTextures, this, gpu), Qt::QueuedConnection);

        ui->AddAction(DAVA::mainWindowKey, placement, action);
        gpuFormatActions.push_back(action);
    };

    for (int gpu = 0; gpu < DAVA::eGPUFamily::GPU_DEVICE_COUNT; ++gpu)
    {
        createGpuAction(static_cast<DAVA::eGPUFamily>(gpu));
    }

    // Separator
    {
        QAction* action = new QAction(nullptr);
        action->setObjectName("originSeparator");
        action->setSeparator(true);

        ui->AddAction(DAVA::mainWindowKey, placement, action);
        gpuFormatActions.push_back(action);
    }

    createGpuAction(DAVA::GPU_ORIGIN);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Reload Texture
    {
        QToolButton* toolButton = new QToolButton();

        QMenu* reloadTextureMenu = new QMenu(toolButton);
        for (QAction* gpuAction : gpuFormatActions)
        {
            reloadTextureMenu->addAction(gpuAction);
        }

        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/reloadtextures.png"), QString(""));
        FieldDescriptor sceneFieldDescr;
        sceneFieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
        sceneFieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
        action->SetStateUpdationFunction(QtAction::Enabled, sceneFieldDescr, [](const DAVA::Any& v) {
            return v.CanCast<SceneData::TSceneType>() && v.Cast<SceneData::TSceneType>().Get() != nullptr;
        });

        FieldDescriptor currentGPUDescr;
        currentGPUDescr.fieldName = DAVA::FastName(TEXTURE_GPU_FIELD_NAME);
        currentGPUDescr.type = DAVA::ReflectedTypeDB::Get<SceneManagerModuleDetail::SceneManagerGlobalData>();
        action->SetStateUpdationFunction(QtAction::Text, currentGPUDescr, [](const DAVA::Any& v)
                                         {
                                             DAVA::String result;
                                             if (v.CanCast<DAVA::eGPUFamily>())
                                             {
                                                 result = GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(v.Cast<DAVA::eGPUFamily>());
                                             }
                                             return result;
                                         });

        toolButton->setMenu(reloadTextureMenu);
        toolButton->setPopupMode(QToolButton::MenuButtonPopup);
        toolButton->setDefaultAction(action);
        toolButton->setMaximumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
        toolButton->setMinimumWidth(ResourceEditor::DEFAULT_TOOLBAR_CONTROL_SIZE_WITH_TEXT);
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setAutoRaise(false);

        AttachWidgetToAction(action, toolButton);

        ActionPlacementInfo placement(CreateToolbarPoint("mainToolBar", InsertionParams(InsertionParams::eInsertionMethod::AfterItem)));
        ui->AddAction(DAVA::mainWindowKey, placement, action);
    }
}

void SceneManagerModule::CreateSceneActions()
{
    using namespace DAVA;

    auto bindStateToProject = [](QtAction* action) {
        FieldDescriptor fieldDescr;
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectManagerData>();
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
            return value.CanCast<DAVA::FilePath>() && !value.Cast<DAVA::FilePath>().IsEmpty();
        });
    };

    auto makeActionBindable = [](QtAction* action, QList<QKeySequence>&& seq, Qt::ShortcutContext ctx = Qt::WindowShortcut) {
        KeyBindableActionInfo info;
        info.blockName = SceneMenuName;
        info.context = ctx;
        info.defaultShortcuts = std::move(seq);
        MakeActionKeyBindable(action, info);
    };

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    QString sceneToolBar = "SceneToolBar";
    // Create Scene top level menu
    {
        QAction* action = new QAction(SceneMenuName, nullptr);
        ui->AddAction(DAVA::mainWindowKey, CreateREMenuPoint(SceneMenuName), action);

        DAVA::ActionPlacementInfo toolbarTogglePlacement(DAVA::CreateMenuPoint(QList<QString>() << "View"
                                                                                                << "Toolbars"));
        ui->DeclareToolbar(DAVA::mainWindowKey, toolbarTogglePlacement, sceneToolBar);
    }

    ActionPlacementInfo menuToolbarPlacementInfo;
    menuToolbarPlacementInfo.AddPlacementPoint(CreateMenuPoint(SceneMenuName));
    menuToolbarPlacementInfo.AddPlacementPoint(CreateToolbarPoint(sceneToolBar));

    ActionPlacementInfo menuPlacementInfo;
    menuPlacementInfo.AddPlacementPoint(CreateMenuPoint(SceneMenuName));

    // Create new level
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/newscene.png"), QString("New level"));
        makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::Key_N, Qt::CTRL + Qt::Key_L) });
        bindStateToProject(action);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::CreateNewScene, this, SceneData::Level), Qt::QueuedConnection);
        ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
    }

    // Create new prefab
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/newscene.png"), QString("New prefab"));
        makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::Key_N, Qt::CTRL + Qt::Key_P) });
        bindStateToProject(action);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::CreateNewScene, this, SceneData::Prefab), Qt::QueuedConnection);
        ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
    }

    // Open
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/openscene.png"), QString("Open"));
        makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::Key_O) });
        bindStateToProject(action);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::OpenScene, this), Qt::QueuedConnection);
        ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
    }

    // Open Scene Quickly Action
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/openscene.png"), QString("Open Scene Quickly"));
        makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O), QKeySequence(Qt::ALT + Qt::SHIFT + Qt::Key_O) }, Qt::ApplicationShortcut);
        bindStateToProject(action);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::OpenSceneQuckly, this), Qt::QueuedConnection);
        ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
    }

    {
      // Save level
      {
      QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/savescene.png"), QString("Save level"));
    makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::Key_S) });

    FieldDescriptor fieldDescr;
    fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
        return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
    });

    fieldDescr.fieldName = DAVA::FastName("editMode");
    action->SetStateUpdationFunction(QtAction::Text, fieldDescr, DAVA::Bind(&SceneManagerModuleDetail::SaveButtonTextFn, DAVA::_1, false));

    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(static_cast<void (SceneManagerModule::*)(bool)>(&SceneManagerModule::SaveScene), this, false));
    ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
}

// Save level
{
    QtAction* action = new QtAction(accessor, QIcon(":/QtIcons/savescene.png"), QString("Save level"));
    makeActionBindable(action, { QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S) });

    FieldDescriptor fieldDescr;
    fieldDescr.fieldName = DAVA::FastName(SceneData::scenePropertyName);
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<SceneData>();
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& value) -> DAVA::Any {
        return value.CanCast<SceneData::TSceneType>() && value.Cast<SceneData::TSceneType>().Get() != nullptr;
    });

    fieldDescr.fieldName = DAVA::FastName("editMode");
    action->SetStateUpdationFunction(QtAction::Text, fieldDescr, DAVA::Bind(&SceneManagerModuleDetail::SaveButtonTextFn, DAVA::_1, true));

    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(static_cast<void (SceneManagerModule::*)(bool)>(&SceneManagerModule::SaveScene), this, true));
    ui->AddAction(mainWindowKey, menuToolbarPlacementInfo, action);
}
}

// Separator
{
    QtActionSeparator* action = new QtActionSeparator("saveSeparator");
    ui->AddAction(mainWindowKey, menuPlacementInfo, action);
}

// Export
{
    QtAction* action = new QtAction(accessor, QString("Export"));
    makeActionBindable(action, {});
    FieldDescriptor fieldDescr;
    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    action->SetStateUpdationFunction(QtAction::Enabled, model, DAVA::FastName("saveToFolderAvailable"), [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return fieldValue.Get<bool>(false);
    });

    connections.AddConnection(action, &QAction::triggered, DAVA::MakeFunction(this, &SceneManagerModule::ExportScene), Qt::QueuedConnection);

    ui->AddAction(mainWindowKey, menuPlacementInfo, action);
}

// Save To Folder
{
    QtAction* action = new QtAction(accessor, QStringLiteral("Save To Folder With Children"));
    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    action->SetStateUpdationFunction(QtAction::Enabled, model, DAVA::FastName("saveToFolderAvailable"), [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return fieldValue.Get<bool>(false);
    });

    ui->AddAction(mainWindowKey, menuPlacementInfo, action);

    QList<QString> menusPath;
    menusPath << SceneMenuName
              << "Save To Folder With Children";

    {
        QAction* action = new QAction(QStringLiteral("Only Original Textures"), nullptr);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::SaveSceneToFolder, this, false), Qt::QueuedConnection);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(menusPath));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    {
        QAction* action = new QAction(QStringLiteral("With Compressed Textures"), nullptr);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SceneManagerModule::SaveSceneToFolder, this, true), Qt::QueuedConnection);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(menusPath));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }
}

// Separator
{
    QtActionSeparator* action = new QtActionSeparator("exportSeparator");
    ui->AddAction(mainWindowKey, menuPlacementInfo, action);
}

// Recent
{
    RecentMenuItems::Params params(DAVA::mainWindowKey, accessor, "Recent scenes");
    params.ui = ui;
    params.menuSubPath << SceneMenuName << "Recent";
    params.predicateFieldDescriptor.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
    params.predicateFieldDescriptor.type = DAVA::ReflectedTypeDB::Get<ProjectManagerData>();
    params.enablePredicate = [](const DAVA::Any& v) -> DAVA::Any {
        return v.CanCast<DAVA::FilePath>() && !v.Cast<DAVA::FilePath>().IsEmpty();
    };
    params.getMaximumCount = [accessor]() {
        return accessor->GetGlobalContext()->GetData<GeneralSettings>()->recentScenesCount;
    };

    recentItems.reset(new RecentMenuItems(std::move(params)));
    recentItems->actionTriggered.Connect([this](const DAVA::String& scenePath) {
        OpenSceneByPath(DAVA::FilePath(scenePath));
    });
}
}

void SceneManagerModule::RegisterOperations()
{
    RegisterOperation(DAVA::CreateFirstSceneOperation.ID, this, &SceneManagerModule::CreateFirstScene);
    RegisterOperation(DAVA::CloseAllScenesOperation.ID, this, &SceneManagerModule::CloseAllScenes);
    RegisterOperation(DAVA::OpenSceneOperation.ID, this, &SceneManagerModule::OpenSceneByPath);
    RegisterOperation(DAVA::AddSceneOperation.ID, this, &SceneManagerModule::AddSceneByPath);
    RegisterOperation(DAVA::SaveCurrentScene.ID, this, static_cast<void (SceneManagerModule::*)()>(&SceneManagerModule::SaveScene));
    RegisterOperation(DAVA::ReloadAllTextures.ID, this, &SceneManagerModule::ReloadAllTextures);
    RegisterOperation(DAVA::ReloadTextures.ID, this, &SceneManagerModule::ReloadTextures);
}

void SceneManagerModule::CreateFirstScene()
{
    using namespace DAVA;
    DataContext* globalCtx = GetAccessor()->GetGlobalContext();
    if (globalCtx->GetData<GlobalSceneSettings>()->openLastScene == true)
    {
        Vector<String> recentScenes = recentItems->Get();
        if (recentScenes.empty() == false)
        {
            FilePath lastOpenedScene = recentScenes[0];

            ProjectManagerData* data = globalCtx->GetData<ProjectManagerData>();
            DVASSERT(data != nullptr);
            FilePath projectPath = data->GetProjectPath();

            if (FilePath::ContainPath(lastOpenedScene, projectPath))
            {
                OpenSceneByPath(lastOpenedScene);
                return;
            }
        }
    }

    CreateNewScene(SceneData::Prefab);
}

void SceneManagerModule::CreateNewScene(DAVA::SceneData::eEditMode mode)
{
    using namespace DAVA;
    ContextManager* contextManager = GetContextManager();

    QString newSceneName = mode == DAVA::SceneData::Prefab ? "Prefab" : "Level";

    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();

    DAVA::FilePath scenePath = QString("%1 %2.%3").arg(newSceneName).arg(++newSceneCounter).arg(newSceneName.toLower()).toStdString();
    sceneData->scene = OpenSceneImpl(scenePath, sceneData->mode);

    CreateSceneProperties(sceneData.get(), true);
    DAVA::Vector<std::unique_ptr<DAVA::TArcDataNode>> initialData;
    initialData.emplace_back(std::move(sceneData));
    DataContext::ContextID newContext = contextManager->CreateContext(std::move(initialData));
    contextManager->ActivateContext(newContext);
}

void SceneManagerModule::OpenScene()
{
    using namespace DAVA;

    DataContext* globalCtx = GetAccessor()->GetGlobalContext();
    ProjectManagerData* data = globalCtx->GetData<ProjectManagerData>();
    DVASSERT(data != nullptr);

    FileDialogParams params;
    params.dir = QString::fromStdString(data->GetDataSource3DPath().GetAbsolutePathname());
    params.filters = QStringLiteral("DAVA Scene V2 (*.sc2 *.level *.prefab)");
    params.title = QStringLiteral("Open scene file");

    QString scenePath = GetUI()->GetOpenFileName(DAVA::mainWindowKey, params);
    OpenSceneByPath(DAVA::FilePath(scenePath.toStdString()));
}

void SceneManagerModule::OpenSceneQuckly()
{
    /// TODO GetFilePath should take UI interface and create widget through it
    DVASSERT(sceneFilesCache);

    DAVA::FileSystem* fileSystem = GetAccessor()->GetEngineContext()->fileSystem;
    if (fileSystem->Exists(cachedPath))
    {
        QString path = FindFileDialog::GetFilePath(GetAccessor(), sceneFilesCache.get(), { "sc2", "level", "prefab" }, GetUI()->GetWindow(DAVA::mainWindowKey));
        if (!path.isEmpty())
        {
            OpenSceneByPath(DAVA::FilePath(path.toStdString()));
        }
    }
}

void SceneManagerModule::OpenSceneByPath(const DAVA::FilePath& scenePath)
{
    using namespace DAVA;

    if (scenePath.IsEmpty())
    {
        return;
    }

    ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::FilePath projectPath(data->GetProjectPath());

    if (!DAVA::FilePath::ContainPath(scenePath, projectPath))
    {
        ModalMessageParams params;
        params.buttons = ModalMessageParams::Ok;
        params.title = QStringLiteral("Open scene error");
        params.message = QString("Can't open scene file outside project path.\n\nScene:\n%1\n\nProject:\n%2")
                         .arg(scenePath.GetAbsolutePathname().c_str())
                         .arg(projectPath.GetAbsolutePathname().c_str());

        GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
        return;
    }

    if (!IsSceneCompatible(scenePath))
    {
        return;
    }

    recentItems->Add(scenePath.GetAbsolutePathname());

    ContextManager* contextManager = GetContextManager();
    ContextAccessor* accessor = GetAccessor();

    // strange logic in my opinion, but...
    // here we check that we have only one scene, this scene was created by default (with index 1 in name) and user did nothing with this scene
    // if we find such scene, we will replace it by loaded scene
    {
        if (accessor->GetContextCount() == 1)
        {
            DataContext* activeContext = accessor->GetActiveContext();
            DAVA::RefPtr<SceneEditor2> scene = activeContext->GetData<SceneData>()->scene;
            if (!scene->IsLoaded() && !scene->IsChanged() && scene->GetScenePath().GetFilename() == "newscene1.sc2")
            {
                contextManager->ActivateContext(DataContext::Empty);
                contextManager->DeleteContext(activeContext->GetID());
            }
        }
    }

    // if scene with "scenePath" have already been opened, we should simply activate it
    {
        FilePath levelPath = scenePath;
        levelPath.ReplaceExtension(".level");
        DataContext::ContextID contextToActivate = DataContext::Empty;
        accessor->ForEachContext([scenePath, levelPath, &contextToActivate](DataContext& ctx)
                                 {
                                     SceneData* data = ctx.GetData<SceneData>();
                                     if (data->scene->GetScenePath() == scenePath ||
                                         data->scene->GetScenePath() == levelPath)
                                     {
                                         DVASSERT(contextToActivate == DataContext::Empty);
                                         contextToActivate = ctx.GetID();
                                     }
                                 });

        if (contextToActivate != DataContext::Empty)
        {
            contextManager->ActivateContext(contextToActivate);
            return;
        }
    }

    UI* ui = GetUI();
    WaitDialogParams waitDlgParams;
    waitDlgParams.message = QString("Opening scene\n%1").arg(scenePath.GetAbsolutePathname().c_str());
    waitDlgParams.needProgressBar = false;

    std::unique_ptr<WaitHandle> waitHandle = ui->ShowWaitDialog(DAVA::mainWindowKey, waitDlgParams);

    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();
    sceneData->scene = OpenSceneImpl(scenePath, sceneData->mode);

    CreateSceneProperties(sceneData.get());
    sceneData->scene->LoadSystemsLocalProperties(sceneData.get()->GetPropertiesRoot(), accessor);

    DAVA::Vector<std::unique_ptr<DAVA::TArcDataNode>> initialData;
    initialData.emplace_back(std::move(sceneData));
    DataContext::ContextID newContext = contextManager->CreateContext(std::move(initialData));
    contextManager->ActivateContext(newContext);
}

void SceneManagerModule::AddSceneByPath(const DAVA::FilePath& scenePath)
{
    using namespace DAVA;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    if ((sceneData != nullptr) && (scenePath.IsEmpty() == false))
    {
        DAVA::RefPtr<SceneEditor2> sceneEditor = sceneData->GetScene();

        UI* ui = GetUI();
        WaitDialogParams waitDlgParams;
        waitDlgParams.message = QString("Add object to scene\n%1").arg(scenePath.GetAbsolutePathname().c_str());
        waitDlgParams.needProgressBar = false;
        std::unique_ptr<WaitHandle> waitHandle = ui->ShowWaitDialog(DAVA::mainWindowKey, waitDlgParams);

        sceneEditor->GetSystem<StructureSystem>()->Add(scenePath);
    }
}

void SceneManagerModule::SaveScene(bool saveAs)
{
    using namespace DAVA;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    DVASSERT(ctx != nullptr);
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene.Get() != nullptr);

    if (!IsSavingAllowed(data))
    {
        return;
    }

    data->scene->SaveSystemsLocalProperties(data->GetPropertiesRoot());

    DAVA::FilePath saveAsPath;
    if (saveAs == true)
    {
        saveAsPath = GetSceneSavePath(data);
    }

    // if it wasn't, we should create properties holder for it
    bool sceneWasLoaded = data->scene->IsLoaded();

    SaveSceneImpl(data, saveAsPath);

    if (sceneWasLoaded == false || saveAs == true)
    {
        CreateSceneProperties(data);
    }
}

void SceneManagerModule::SaveScene()
{
    SaveScene(false);
}

void SceneManagerModule::CreateSceneProperties(DAVA::SceneData* const data, bool sceneIsTemp)
{
    DAVA::FilePath dirPath, fileName;
    GetPropertiesFilePath(data->scene->GetScenePath(), dirPath, fileName, sceneIsTemp);
    data->CreatePropertiesRoot(GetAccessor()->GetEngineContext()->fileSystem, dirPath, fileName);
}

void SceneManagerModule::GetPropertiesFilePath(const DAVA::FilePath& scenePath, DAVA::FilePath& path,
                                               DAVA::FilePath& fileName, bool sceneIsTemp)
{
    using namespace DAVA;

    // documents directory
    FileSystem* fs = GetAccessor()->GetEngineContext()->fileSystem;
    FilePath documentRoot = fs->GetCurrentDocumentsDirectory();

    // scene properties subdirectory
    static const String propertiesSubDir = "SceneProperties";

    String projectHash, relativeSceneDirPath;
    if (sceneIsTemp == true)
    {
        // default scene properties location
        projectHash = "Default";
        relativeSceneDirPath = scenePath.GetFilename() + "/";
        fileName = scenePath.GetFilename();
    }
    else
    {
        // scene name and relative path
        ProjectManagerData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
        FilePath projectPath(projectData->GetProjectPath());
        fileName = FilePath(scenePath.GetFilename());
        relativeSceneDirPath = FilePath(scenePath.GetDirectory()).GetRelativePathname(projectPath);

        // project path hash
        MD5::MD5Digest projectMD5;
        String absPath = projectPath.GetAbsolutePathname();
        MD5::ForData(reinterpret_cast<const uint8*>(absPath.c_str()),
                     static_cast<uint32>(absPath.size()),
                     projectMD5);
        projectHash = MD5::HashToString(projectMD5);
    }

    // final path
    path = FilePath(documentRoot.GetAbsolutePathname()
                    + "/" + propertiesSubDir
                    + "/" + projectHash
                    + "/" + relativeSceneDirPath);
}

void SceneManagerModule::SaveSceneToFolder(bool compressedTextures)
{
    using namespace DAVA;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    DVASSERT(ctx != nullptr);
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene.Get() != nullptr);

    DAVA::RefPtr<SceneEditor2> scene = data->scene;

    if (!IsSavingAllowed(data))
    {
        return;
    }

    UI* ui = GetUI();

    const DAVA::FilePath& scenePathname = scene->GetScenePath();
    if (scenePathname.IsEmpty() || scenePathname.GetType() == DAVA::FilePath::PATH_IN_MEMORY || !scene->IsLoaded())
    {
        ModalMessageParams params;
        params.title = QStringLiteral("Save to folder");
        params.message = QStringLiteral("Can't save not saved scene.");
        params.buttons = ModalMessageParams::Ok;
        ui->ShowModalMessage(DAVA::mainWindowKey, params);
        return;
    }

    DirectoryDialogParams dirDlgParams;
    dirDlgParams.dir = QString("/");
    dirDlgParams.title = QString("Open Folder");
    QString path = ui->GetExistingDirectory(DAVA::mainWindowKey, dirDlgParams);
    if (path.isEmpty())
    {
        return;
    }

    WaitDialogParams waitDlgParams;
    waitDlgParams.needProgressBar = false;
    waitDlgParams.message = QStringLiteral("Save with Children.\nPlease wait...");
    ui->ShowWaitDialog(DAVA::mainWindowKey, waitDlgParams);

    DAVA::FilePath folder(path.toStdString());
    folder.MakeDirectoryPathname();

    SceneSaver sceneSaver;
    sceneSaver.SetInFolder(scene->GetScenePath().GetDirectory());
    sceneSaver.SetOutFolder(folder);
    sceneSaver.EnableCopyConverted(compressedTextures);

    { //tags
        ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
        if (data->GetEditorConfig()->HasProperty("Tags"))
        {
            DAVA::Vector<DAVA::String> projectTags = data->GetEditorConfig()->GetComboPropertyValues("Tags");
            projectTags.insert(projectTags.begin(), "");
            sceneSaver.SetTags(projectTags);
        }
        else
        {
            sceneSaver.SetTags({ "" });
        }
    }

    SceneEditor2* sceneForSaving = scene->CreateCopyForExport();
    sceneSaver.SaveScene(sceneForSaving, scene->GetScenePath());
    sceneForSaving->Release();
}

void SceneManagerModule::ExportScene()
{
    using namespace DAVA;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    DVASSERT(ctx != nullptr);
    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene.Get() != nullptr);

    DAVA::RefPtr<SceneEditor2> scene = data->scene;

    ExportSceneDialog dlg;
    dlg.exec();
    if (dlg.result() == QDialog::Accepted && SaveTileMaskInScene(scene))
    {
        ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
        DVASSERT(data != nullptr);
        const DAVA::FilePath& projectPath = data->GetProjectPath();
        DAVA::FilePath dataSourceFolder = projectPath + "DataSource/3d/";

        {
            WaitDialogParams params;
            params.needProgressBar = false;
            params.message = QStringLiteral("Scene exporting.\nPlease wait...");
            std::unique_ptr<WaitHandle> waitHandle = GetUI()->ShowWaitDialog(DAVA::mainWindowKey, params);

            SceneExporter::Params exportingParams;
            exportingParams.outputs.emplace_back(dlg.GetDataFolder(), dlg.GetGPUs(), dlg.GetQuality(), dlg.GetUseHDTextures());
            exportingParams.dataSourceFolder = dataSourceFolder;
            exportingParams.optimizeOnExport = dlg.GetOptimizeOnExport();
            exportingParams.filenamesTag = dlg.GetFilenamesTag();
            scene->Export(exportingParams);
        }

        TextureAssetLoader* loader = GetEngineContext()->assetManager->GetAssetLoader<TextureAssetLoader>();
        ReloadAllTextures(loader->GetPrimaryGPUForLoading());
    }
}

void SceneManagerModule::CloseAllScenes(bool needSavingReqiest)
{
    using namespace DAVA;
    DAVA::Vector<DataContext::ContextID> contexts;
    GetAccessor()->ForEachContext([&contexts](DataContext& context)
                                  {
                                      contexts.push_back(context.GetID());
                                  });

    for (DataContext::ContextID id : contexts)
    {
        if (CloseSceneImpl(id, needSavingReqiest) == false)
        {
            return;
        }
    }
}

void SceneManagerModule::ReloadTextures(const DAVA::Vector<DAVA::Asset<DAVA::Texture>>& textures)
{
    // GFX_COMPLETE
    /*using namespace DAVA;

    CommonInternalSettings* settings = GetAccessor()->GetGlobalContext()->GetData<CommonInternalSettings>();

    DAVA::eGPUFamily gpuFormat = settings->textureViewGPU;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    DAVA::RefPtr<SceneEditor2> scene = activeContext->GetData<SceneData>()->scene;

    DAVA::Set<DAVA::NMaterial*> materials;
    SceneHelper::EnumerateMaterials(scene.Get(), materials);

    int progress = 0;
    WaitDialogParams params;
    std::unique_ptr<WaitHandle> waitHandle;

    if (textures.size() > 1)
    {
        params.needProgressBar = true;
        params.message = "Reloading textures.";
        params.min = 0;
        params.max = static_cast<int>(textures.size());
        waitHandle = GetUI()->ShowWaitDialog(DAVA::mainWindowKey, params);
    }

    DAVA::AssetManager* assetManager = GetEngineContext()->assetManager;
    DAVA::TextureAssetLoader* loader = assetManager->GetAssetLoader<DAVA::TextureAssetLoader>();
    loader->SetGPULoadingOrder({ gpuFormat });

    for (DAVA::Asset<DAVA::Texture> tex : textures)
    {
        DAVA::TextureDescriptor* descriptor = tex->GetDescriptor();

        if (textures.size() > 1)
        {
            QString message = QString("Reloading texture:%1");
            message = message.arg(descriptor->GetSourceTexturePathname().GetAbsolutePathname().c_str());
            waitHandle->SetMessage(message);
            waitHandle->SetProgressValue(progress++);
        }

        assetManager->ReloadAsset(tex->GetKey());
        TextureCache::Instance()->clearOriginal(descriptor);
        TextureCache::Instance()->clearThumbnail(descriptor);

        for (auto mat : materials)
        {
            if (mat->ContainsTexture(tex))
                mat->InvalidateTextureBindings();
        }

        TextureBrowser::Instance()->UpdateTexture(tex);
    }*/
}

void SceneManagerModule::ReloadAllTextures(DAVA::eGPUFamily gpu)
{
    // GFX_COMPLETE
    /*using namespace DAVA;
    if (SaveTileMaskInAllScenes())
    {
        CommonInternalSettings* settings = GetAccessor()->GetGlobalContext()->GetData<CommonInternalSettings>();
        settings->textureViewGPU = gpu;
        DAVA::Texture::SetGPULoadingOrder({ gpu });

        SceneHelper::TextureCollector collector;
        DAVA::Set<DAVA::NMaterial*> allSceneMaterials;

        ContextAccessor* accessor = GetAccessor();

        accessor->ForEachContext([&collector, &allSceneMaterials](DataContext& ctx)
                                 {
                                     SceneData* data = ctx.GetData<SceneData>();
                                     SceneHelper::EnumerateSceneTextures(data->scene.Get(), collector);
                                     SceneHelper::EnumerateMaterials(data->scene.Get(), allSceneMaterials);
                                 });

        DAVA::TexturesMap& allScenesTextures = collector.GetTextures();
        if (!allScenesTextures.empty())
        {
            int progress = 0;
            WaitDialogParams params;
            params.needProgressBar = true;
            params.message = "Reloading textures.";
            params.min = 0;
            params.max = static_cast<DAVA::uint32>(allScenesTextures.size());

            std::unique_ptr<WaitHandle> waitHandle = GetUI()->ShowWaitDialog(DAVA::mainWindowKey, params);

            DAVA::TexturesMap::const_iterator it = allScenesTextures.begin();
            DAVA::TexturesMap::const_iterator end = allScenesTextures.end();

            for (; it != end; ++it)
            {
                QString message = QString("Reloading texture:%1");
                message = message.arg(it->first.GetAbsolutePathname().c_str());
                waitHandle->SetMessage(message);
                waitHandle->SetProgressValue(progress++);

                it->second->ReloadAs(gpu);
            }

            TextureCache::Instance()->ClearCache();
        }

        accessor->ForEachContext([](DataContext& ctx)
                                 {
                                     SceneData* data = ctx.GetData<SceneData>();
                                     data->scene->GetSystem<DAVA::EditorVegetationSystem>()->ReloadVegetation();
                                 });

        DAVA::Sprite::ReloadSprites(gpu);
        RestartParticles();
    }*/
}

void SceneManagerModule::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    DVASSERT(sceneFilesCache);

    DAVA::ContextAccessor* accessor = GetAccessor();

    DAVA::FileSystem* fileSystem = accessor->GetEngineContext()->fileSystem;
    if (fileSystem->Exists(cachedPath))
    {
        sceneFilesCache->UntrackDirectory(QString::fromStdString(cachedPath.GetAbsolutePathname()));
    }

    cachedPath = "";

    if (projectPath.CanCast<DAVA::FilePath>())
    {
        DAVA::FilePath path = projectPath.Cast<DAVA::FilePath>();
        if (path.IsEmpty() == false)
        {
            DAVA::ProjectManagerData* projectData = accessor->GetGlobalContext()->GetData<DAVA::ProjectManagerData>();
            DVASSERT(projectData != nullptr);

            cachedPath = projectData->GetDataSource3DPath();
        }
    }

    if (fileSystem->Exists(cachedPath))
    {
        sceneFilesCache->TrackDirectory(QString::fromStdString(cachedPath.GetAbsolutePathname()));
    }
}

bool SceneManagerModule::OnCloseSceneRequest(DAVA::uint64 id)
{
    return CloseSceneImpl(id, true);
}

void SceneManagerModule::OnDragEnter(QObject* target, QDragEnterEvent* event)
{
    DefaultDragHandler(target, event);
}

void SceneManagerModule::OnDragMove(QObject* target, QDragMoveEvent* event)
{
    DefaultDragHandler(target, event);
}

void SceneManagerModule::OnDrop(QObject* target, QDropEvent* event)
{
    using namespace DAVA;
    bool isRenderWidget = qobject_cast<DAVA::RenderWidget*>(target) != nullptr;
    if (IsValidMimeData(event, isRenderWidget) == false)
    {
        return;
    }

    event->setAccepted(true);

    const QMimeData* data = event->mimeData();
    DAVA::Vector<DAVA::FilePath> files;
    foreach (QUrl url, data->urls())
    {
        QString path = url.toLocalFile();
        QString suffix = QFileInfo(path).suffix();
        if (suffix == "prefab")
        {
            files.push_back(DAVA::FilePath(path.toStdString()));
        }
        else if (suffix == "level" && isRenderWidget == false)
        {
            files.push_back(DAVA::FilePath(path.toStdString()));
        }
    }

    DataContext* ctx = GetAccessor()->GetActiveContext();
    if (ctx != nullptr && isRenderWidget == true)
    {
        SceneData* data = ctx->GetData<SceneData>();
        DAVA::Vector3 pos;

        // check if there is intersection with landscape. ray from camera to mouse pointer
        // if there is - we should move opening scene to that point
        DAVA::SceneCollisionSystem* collisionSystem = data->scene->GetSystem<DAVA::SceneCollisionSystem>();

        if (!collisionSystem->LandRayTestFromCamera(pos))
        {
            DAVA::Landscape* landscape = collisionSystem->GetCurrentLandscape();
            if (landscape != nullptr && landscape->GetHeightmap() != nullptr && landscape->GetHeightmap()->Size() > 0)
            {
                collisionSystem->GetCurrentLandscape()->PlacePoint(DAVA::Vector3(), pos);
            }
        }

        WaitDialogParams params;
        params.message = QStringLiteral("Adding object to scene");
        params.min = 0;
        params.max = static_cast<DAVA::uint32>(files.size());

        std::unique_ptr<WaitHandle> waitHandle = GetUI()->ShowWaitDialog(DAVA::mainWindowKey, params);
        for (size_t i = 0; i < files.size(); ++i)
        {
            const DAVA::FilePath& path = files[i];
            if (IsSceneCompatible(path))
            {
                data->scene->GetSystem<DAVA::StructureSystem>()->Add(path, pos);
                waitHandle->SetProgressValue(static_cast<DAVA::uint32>(i));
            }
        }
    }
    else
    {
        for (const DAVA::FilePath& path : files)
        {
            OpenSceneByPath(path);
        }
    }
}

///////////////////////////////
///           Helpers       ///
///////////////////////////////
bool SceneManagerModule::CanCloseScene(DAVA::SceneData* data)
{
    using namespace DAVA;

    DAVA::int32 toolsFlags = data->scene->GetEnabledTools();
    if (!data->scene->IsChanged())
    {
        if (toolsFlags)
        {
            data->scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);
        }
        return true;
    }

    if (!IsSavingAllowed(data))
    {
        return false;
    }

    UI* ui = GetUI();
    ModalMessageParams params;
    params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Cancel;
    params.defaultButton = ModalMessageParams::Cancel;
    params.message = "Do you want to save changes, made to scene?";
    params.title = "Scene was changed";

    ModalMessageParams::Button answer = ui->ShowModalMessage(DAVA::mainWindowKey, params);

    if (answer == ModalMessageParams::Cancel)
    {
        return false;
    }

    if (answer == ModalMessageParams::No)
    {
        if (toolsFlags)
        {
            data->scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, false);
        }
        return true;
    }

    if (toolsFlags != 0)
    {
        data->scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL, true);
    }

    if (!SaveSceneImpl(data))
    {
        return false;
    }

    return true;
}

DAVA::RefPtr<DAVA::SceneEditor2> SceneManagerModule::OpenSceneImpl(const DAVA::FilePath& scenePath, DAVA::SceneData::eEditMode& mode)
{
    using namespace DAVA;

    DAVA::RefPtr<SceneEditor2> scene(new SceneEditor2());
    FilePath newScenePath = scenePath;

    ContextAccessor* accessor = GetAccessor();
    const DAVA::EngineContext* engineCtx = accessor->GetEngineContext();

    if (engineCtx->fileSystem->Exists(scenePath))
    {
        if (scenePath.GetExtension() == ".sc2")
        {
            newScenePath.ReplaceExtension(".level");
            DAVA::SceneFileV2::eError sceneWasLoaded = scene->LoadScene(scenePath);
            if (sceneWasLoaded != DAVA::SceneFileV2::ERROR_NO_ERROR)
            {
                ModalMessageParams params;
                params.buttons = ModalMessageParams::Ok;
                params.message = QStringLiteral("Unexpected opening error. See logs for more info.");
                params.title = QStringLiteral("Open scene error.");

                GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
            }
            else
            {
                DAVA::SceneFileConverter::ConvertSceneToLevelFormat(scene.Get(), scenePath.GetDirectory());
            }

            mode = SceneData::Level;
        }
        else if (scenePath.GetExtension() == ".level")
        {
            mode = SceneData::Level;
            scene->streamingSystem->LoadLevel(scenePath);
        }
        else if (scenePath.GetExtension() == ".prefab")
        {
            mode = SceneData::Prefab;
            scene->LoadAsPrefab(scenePath);
        }
    }
    scene->SetScenePath(newScenePath);
    scene->EnableEditorSystems();

    return scene;
}

bool SceneManagerModule::SaveSceneImpl(DAVA::SceneData* sceneData, const DAVA::FilePath& scenePath)
{
    DAVA::FilePath pathToSaveScene = scenePath;
    if (pathToSaveScene.IsEmpty())
    {
        if (sceneData->scene->IsLoaded())
        {
            DAVA::FilePath currentScenePath = sceneData->scene->GetScenePath();
            DVASSERT(!currentScenePath.IsEmpty());
            if (!sceneData->scene->IsChanged())
            {
                return false;
            }

            pathToSaveScene = sceneData->scene->GetScenePath();
        }
        else
        {
            pathToSaveScene = GetSceneSavePath(sceneData);
        }
    }

    if (pathToSaveScene.IsEmpty())
    {
        return false;
    }

    if (GetAccessor()->GetGlobalContext()->GetData<DAVA::GlobalSceneSettings>()->saveEmitters == true)
    {
        sceneData->scene->SaveEmitters(DAVA::MakeFunction(this, &SceneManagerModule::SaveEmitterFallback));
    }

    {
        DAVA::SceneEditor2::SceneSaveGuard guard(sceneData->scene.Get(), false);
        bool sceneSaved = false;
        DAVA::AssetManager* assetManager = DAVA::GetEngineContext()->assetManager;
        if (sceneData->mode == DAVA::SceneData::Prefab)
        {
            DVASSERT(pathToSaveScene.GetExtension() == ".prefab");
            DAVA::Prefab::PathKey key(pathToSaveScene);
            sceneSaved = assetManager->SaveAssetFromData(static_cast<DAVA::Scene*>(sceneData->scene.Get()), key);
        }
        else
        {
            DVASSERT(pathToSaveScene.GetExtension() == ".prefab");
            //DAVA::Level::PathKey key(pathToSaveScene);
            //sceneSaved = assetManager->SaveAssetFromData(static_cast<DAVA::Scene*>(sceneData->scene.Get()), key);
        }

        if (sceneSaved == true)
        {
            guard.SavedSuccesfull(pathToSaveScene);
        }
        else
        {
            using namespace DAVA;
            UI* ui = GetUI();
            ModalMessageParams params;
            params.buttons = ModalMessageParams::Ok;
            params.title = QStringLiteral("Save error");
            params.message = QStringLiteral("An error occurred while saving the scene.See log for more info.");
            ui->ShowModalMessage(DAVA::mainWindowKey, params);
            return false;
        }
    }

    recentItems->Add(pathToSaveScene.GetAbsolutePathname());
    return true;
}

DAVA::FilePath SceneManagerModule::GetSceneSavePath(DAVA::SceneData* sceneData)
{
    DVASSERT(sceneData->scene.Get() != nullptr);

    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    const DAVA::EngineContext* engineContext = accessor->GetEngineContext();

    DAVA::FilePath initialPath = sceneData->scene->GetScenePath();
    if (!engineContext->fileSystem->Exists(initialPath))
    {
        ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
        DVASSERT(data != nullptr);
        DAVA::FilePath dataSourcePath = data->GetDataSource3DPath();
        initialPath = dataSourcePath.MakeDirectoryPathname() + sceneData->scene->GetScenePath().GetFilename();
    }

    FileDialogParams params;
    params.dir = QString::fromStdString(initialPath.GetAbsolutePathname());
    params.title = QStringLiteral("Save scene as");
    if (sceneData->mode == SceneData::Prefab)
    {
        params.filters = "DAVA Prefab (*.prefab)";
    }
    else
    {
        params.filters = "DAVA level (*.level)";
    }
    QString saveScenePath = GetUI()->GetSaveFileName(DAVA::mainWindowKey, params);
    return DAVA::FilePath(saveScenePath.toStdString());
}

DAVA::FilePath SceneManagerModule::SaveEmitterFallback(const DAVA::String& entityName, const DAVA::String& emitterName)
{
    DAVA::CommonInternalSettings* settings = GetAccessor()->GetGlobalContext()->GetData<DAVA::CommonInternalSettings>();
    DAVA::FilePath defaultPath = settings->emitterSaveDir;
    if (defaultPath.IsEmpty())
    {
        DAVA::ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<DAVA::ProjectManagerData>();
        DVASSERT(data != nullptr);
        defaultPath = data->GetParticlesConfigPath().GetAbsolutePathname();
    }

    DAVA::FileSystem* fileSystem = GetAccessor()->GetEngineContext()->fileSystem;
    fileSystem->CreateDirectory(defaultPath.GetDirectory(), true);

    DAVA::String emitterYamlName = entityName + "_" + emitterName + ".yaml";

    using namespace DAVA;
    FileDialogParams params;
    params.dir = QString::fromStdString((defaultPath + emitterYamlName).GetAbsolutePathname());
    params.title = QString("Save Particle Emitter %1").arg(emitterName.c_str());
    params.filters = QStringLiteral("YAML File (*.yaml)");

    QString savePath = GetUI()->GetSaveFileName(DAVA::mainWindowKey, params);
    DAVA::FilePath result(savePath.toStdString());
    if (!result.IsEmpty())
    {
        settings->emitterSaveDir = result;
    }

    return result;
}

bool SceneManagerModule::IsSceneCompatible(const DAVA::FilePath& scenePath)
{
    if (scenePath.GetExtension() == ".level" || scenePath.GetExtension() == ".prefab")
        return true;

    DAVA::VersionInfo::SceneVersion sceneVersion = DAVA::SceneFileV2::LoadSceneVersion(scenePath);
    DAVA::VersionInfo* versionInfo = DAVA::VersionInfo::Instance();

    if (sceneVersion.IsValid())
    {
        DAVA::VersionInfo::eStatus status = versionInfo->TestVersion(sceneVersion);
        const DAVA::uint32 curVersion = versionInfo->GetCurrentVersion().version;

        using namespace DAVA;
        ModalMessageParams params;
        params.title = QStringLiteral("Compatibility error");

        switch (status)
        {
        case DAVA::VersionInfo::COMPATIBLE:
        {
            const DAVA::String& branches = DAVA::VersionInfo::Instance()->UnsupportedTagsMessage(sceneVersion);
            params.message = QString("Scene was created with older version or another branch of ResourceEditor. Saving scene will broke compatibility.\nScene version: %1 (required %2)\n\nNext tags will be added:\n%3\n\nContinue opening?").arg(sceneVersion.version).arg(curVersion).arg(branches.c_str());
            params.buttons = ModalMessageParams::Open | ModalMessageParams::Cancel;
            params.defaultButton = ModalMessageParams::Open;
            ModalMessageParams::Button result = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);

            if (result != ModalMessageParams::Open)
            {
                return false;
            }
            break;
        }
        case DAVA::VersionInfo::INVALID:
        {
            const DAVA::String& branches = DAVA::VersionInfo::Instance()->NoncompatibleTagsMessage(sceneVersion);
            params.message = QString("Scene was created with incompatible version or branch of ResourceEditor.\nScene version: %1 (required %2)\nNext tags aren't implemented in current branch:\n%3").arg(sceneVersion.version).arg(curVersion).arg(branches.c_str());
            GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
            return false;
        }
        default:
            break;
        }
    }

    return true;
}

bool SceneManagerModule::SaveTileMaskInAllScenes()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DVASSERT(accessor->GetActiveContext() != nullptr);
    DataContext::ContextID activeContextID = accessor->GetActiveContext()->GetID();

    SCOPE_EXIT
    {
        GetContextManager()->ActivateContext(activeContextID);
    };

    DAVA::Vector<DataContext::ContextID> contexts;

    accessor->ForEachContext([&contexts](DataContext& ctx)
                             {
                                 contexts.push_back(ctx.GetID());
                             });

    ModalMessageParams::Button answer = ModalMessageParams::NoButton;
    for (DataContext::ContextID contextID : contexts)
    {
        SceneData* sceneData = accessor->GetContext(contextID)->GetData<SceneData>();
        DVASSERT(sceneData != nullptr);
        const RECommandStack* cmdStack = sceneData->scene->GetCommandStack();
        if (cmdStack->IsUncleanCommandExists<DAVA::ModifyTilemaskCommand>())
        {
            if (answer != ModalMessageParams::YesToAll && answer != ModalMessageParams::NoToAll)
            {
                GetContextManager()->ActivateContext(contextID);

                ModalMessageParams params;
                params.message = QString("%1 has unsaved tilemask changes.\nDo you want to save?").arg(sceneData->scene->GetScenePath().GetFilename().c_str());
                params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Cancel;
                if (contexts.size() > 1)
                {
                    params.buttons |= (ModalMessageParams::YesToAll | ModalMessageParams::NoToAll);
                }
                params.defaultButton = ModalMessageParams::Cancel;
                answer = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
            }

            if (answer == ModalMessageParams::Cancel)
            {
                return false;
            }

            sceneData->scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

            DAVA::LandscapeEditorDrawSystem* drawSystem = sceneData->scene->GetSystem<DAVA::LandscapeEditorDrawSystem>();
            if (answer == ModalMessageParams::Yes || answer == ModalMessageParams::YesToAll)
            {
                drawSystem->SaveTileMaskTexture();
            }

            drawSystem->ResetTileMaskTextures();
            sceneData->scene->RemoveCommands<DAVA::ModifyTilemaskCommand>();
        }
    }

    return true;
}

bool SceneManagerModule::SaveTileMaskInScene(DAVA::RefPtr<DAVA::SceneEditor2> scene)
{
    using namespace DAVA;
    const RECommandStack* cmdStack = scene->GetCommandStack();
    if (cmdStack->IsUncleanCommandExists<DAVA::ModifyTilemaskCommand>())
    {
        ModalMessageParams params;
        params.message = QString("%1 has unsaved tilemask changes.\nDo you want to save?").arg(scene->GetScenePath().GetFilename().c_str());
        params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Cancel;
        params.defaultButton = ModalMessageParams::Cancel;

        ModalMessageParams::Button result = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);

        if (result == ModalMessageParams::Cancel)
        {
            return false;
        }

        DAVA::LandscapeEditorDrawSystem* drawSystem = scene->GetSystem<DAVA::LandscapeEditorDrawSystem>();
        scene->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);
        if (result == ModalMessageParams::Yes)
        {
            drawSystem->SaveTileMaskTexture();
        }

        drawSystem->ResetTileMaskTextures();
        scene->RemoveCommands<DAVA::ModifyTilemaskCommand>();
    }

    return true;
}

bool SceneManagerModule::CloseSceneImpl(DAVA::uint64 id, bool needSavingRequest)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();

    DataContext* context = accessor->GetContext(id);
    if (context == nullptr)
    {
        // This situation is undefined behavior and Assert should signalize about this
        DVASSERT(false);
        // But if we didn't find context with "id", that means that context already closed, or never exists
        // so we will not discard any user changes if simple say "Yes, scene was successfully closed"
        return true;
    }

    SceneData* data = context->GetData<SceneData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->scene.Get() != nullptr);

    data->scene->SaveSystemsLocalProperties(data->GetPropertiesRoot());

    if (needSavingRequest == false || CanCloseScene(data))
    {
        contextManager->DeleteContext(id);
        return true;
    }

    return false;
}

void SceneManagerModule::RestartParticles()
{
    GetAccessor()->ForEachContext([](DAVA::DataContext& ctx)
                                  {
                                      DAVA::SceneData* data = ctx.GetData<DAVA::SceneData>();
                                      data->scene->GetSystem<DAVA::EditorParticlesSystem>()->RestartParticleEffects();
                                  });
}

bool SceneManagerModule::IsSavingAllowed(DAVA::SceneData* sceneData)
{
    QString message;
    bool result = sceneData->IsSavingAllowed(&message);
    if (result == false)
    {
        using namespace DAVA;
        ModalMessageParams params;
        params.message = message;
        params.buttons = ModalMessageParams::Ok;
        params.title = "Saving is not allowed";
        GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
    }

    return result;
}

void SceneManagerModule::DefaultDragHandler(QObject* target, QDropEvent* event)
{
    bool isRenderWidget = qobject_cast<DAVA::RenderWidget*>(target) != nullptr;
    if (!IsValidMimeData(event, isRenderWidget))
    {
        return;
    }

    if (isRenderWidget)
    {
        event->setDropAction(Qt::LinkAction);
    }
    else
    {
        event->setDropAction(Qt::CopyAction);
    }

    event->setAccepted(true);
}

bool SceneManagerModule::IsValidMimeData(QDropEvent* event, bool isRenderWidget)
{
    const QMimeData* data = event->mimeData();
    if (data->hasUrls())
    {
        foreach (const QUrl& url, data->urls())
        {
            QString suffix = QFileInfo(url.toLocalFile()).suffix();
            if (suffix == "prefab")
            {
                return true;
            }

            if (suffix == "level" && isRenderWidget == false)
            {
                return true;
            }
        }
    }

    event->setDropAction(Qt::IgnoreAction);
    event->accept();
    return false;
}

void SceneManagerModule::DeleteSelection()
{
    using namespace DAVA;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> scene = ctx->GetData<SceneData>()->scene;
    DAVA::RemoveSelection(scene.Get());
}

void SceneManagerModule::MoveToSelection()
{
    using namespace DAVA;
    DataContext* ctx = GetAccessor()->GetActiveContext();
    if (ctx == nullptr)
    {
        return;
    }

    DAVA::RefPtr<SceneEditor2> scene = ctx->GetData<SceneData>()->scene;
    scene->GetSystem<DAVA::SceneCameraSystem>()->MoveToSelection(scene->GetSystem<DAVA::SelectionSystem>()->GetSelection());
}

bool SceneManagerModule::SaveToFolderAvailable() const
{
    if (GetAccessor()->GetActiveContext() != nullptr)
    {
        DAVA::FileSystem* fs = DAVA::GetEngineContext()->fileSystem;
        return fs->GetFilenamesTag().empty() == true;
    }

    return false;
}
