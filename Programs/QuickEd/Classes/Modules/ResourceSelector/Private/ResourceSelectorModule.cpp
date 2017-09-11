#include "Modules/ResourceSelector/ResourceSelectorModule.h"
#include "Modules/ProjectModule/ProjectData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/Common.h>
#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Engine/EngineContext.h>
#include <UI/UIControlSystem.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Utils/Random.h>

#include <memory>

namespace ResourceSelectorModuleDetails
{
const QList<QString> menuResources = QList<QString>{ "View", "Resources" };

class ResourceSelectorData : public DAVA::TArc::SettingsNode
{
public:
    DAVA::int32 preferredMode = 0;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ResourceSelectorData, DAVA::TArc::SettingsNode)
    {
        DAVA::ReflectionRegistrator<ResourceSelectorData>::Begin()[DAVA::M::HiddenField()]
        .ConstructorByPointer()
        .Field("preferredMode", &ResourceSelectorData::preferredMode)[DAVA::M::HiddenField()]
        .End();
    }
};

DAVA::String GetNameFromGfxFolder(const DAVA::String& folder)
{
    return folder.substr(2, folder.length() - 3); // because of "./Gfx/" ... "./Gfx2/"
}
}

ResourceSelectorModule::ResourceSelectorModule()
{
    using namespace DAVA;
    using namespace ResourceSelectorModuleDetails;
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ResourceSelectorData);

    //first VCS initialization
    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    vcs->UnregisterAllAvailableResourceSizes();
    vcs->SetVirtualScreenSize(1, 1);
    vcs->RegisterAvailableResourceSize(1, 1, "Gfx");
}

void ResourceSelectorModule::PostInit()
{
    using namespace DAVA;
    using namespace ResourceSelectorModuleDetails;

    //create data wrapper
    projectDataWrapper = GetAccessor()->CreateWrapper(ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);

    Window* primaryWindow = GetPrimaryWindow();
    DVASSERT(primaryWindow != nullptr);

    primaryWindow->dpiChanged.Connect([this](Window* /*w*/, float32 /*dpi*/)
                                      {
                                          delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ResourceSelectorModule::OnGraphicsSettingsChanged));
                                      });

    primaryWindow->sizeChanged.Connect([this](Window* /*w*/, Size2f windowSize, Size2f /* surfaceSize */)
                                       {
                                           OnWindowResized(windowSize);
                                           delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ResourceSelectorModule::OnGraphicsSettingsChanged));
                                       });
}

void ResourceSelectorModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    DVASSERT(wrapper == projectDataWrapper);

    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();

    UI* ui = GetUI();

    if (gfxActionPlacementName.empty() == false)
    { // remove old actions and old menu items
        for (const QString& actionName : gfxActionPlacementName)
        {
            ActionPlacementInfo placementInfo(CreateMenuPoint(QStringList() << menuResources << actionName));
            ui->RemoveAction(DAVA::TArc::mainWindowKey, placementInfo);
        }
        gfxActionPlacementName.clear();
    }

    if (projectData != nullptr)
    { //project was opened or changed
        const Vector<ProjectData::GfxDir>& gfxDirectories = projectData->GetGfxDirectories();
        if (gfxDirectories.empty() == false)
        {
            { // create menu Resources - container for Gfx Actions
                QtAction* action = new QtAction(GetAccessor(), "Resources");
                ActionPlacementInfo placementInfo;
                placementInfo.AddPlacementPoint(CreateMenuPoint("View", { InsertionParams::eInsertionMethod::BeforeItem, "Toolbars" }));
                GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
            }

            int32 count = static_cast<int32>(gfxDirectories.size());
            { // create actions in menu Resources
                QString prevActionName = "";
                for (int32 ia = 0; ia < count; ++ia)
                {
                    const ProjectData::GfxDir& gfx = gfxDirectories[ia];
                    QString actionName = QString::fromStdString(GetNameFromGfxFolder(gfx.directory.relative));
                    CreateAction(actionName, prevActionName, ia);
                    prevActionName = actionName;
                }

                CreateAction("Auto", prevActionName, count);
            }

            { // Enable last selected settings
                ResourceSelectorData* selectorData = globalContext->GetData<ResourceSelectorData>();
                DVASSERT(selectorData != nullptr);
                if (selectorData->preferredMode > count)
                {
                    selectorData->preferredMode = 0; //somebody deleted gfx folder, we should select default value
                }
                OnGfxSelected(selectorData->preferredMode);
            }
        }
    }
}

void ResourceSelectorModule::OnGfxSelected(DAVA::int32 gfxMode)
{
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    ResourceSelectorData* selectorData = GetAccessor()->GetGlobalContext()->GetData<ResourceSelectorData>();
    DVASSERT(selectorData != nullptr);
    selectorData->preferredMode = gfxMode;

    RegisterGfxFolders();

    delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ResourceSelectorModule::ReloadSpritesImpl));
}

void ResourceSelectorModule::RegisterGfxFolders()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    ResourceSelectorData* selectorData = globalContext->GetData<ResourceSelectorData>();
    DVASSERT(selectorData != nullptr);
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);

    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;

    const Vector<ProjectData::GfxDir>& gfxDirectories = projectData->GetGfxDirectories();
    int32 gfxCount = static_cast<int32>(gfxDirectories.size());
    int32 gfxMode = selectorData->preferredMode;
    Size2i currentSize = vcs->GetVirtualScreenSize();

    vcs->UnregisterAllAvailableResourceSizes();
    if (gfxMode < gfxCount)
    {
        String name = GetNameFromGfxFolder(gfxDirectories[gfxMode].directory.relative);
        vcs->RegisterAvailableResourceSize(currentSize.dx, currentSize.dy, name);
    }
    else if (gfxMode == gfxCount)
    {
        for (const ProjectData::GfxDir& dir : gfxDirectories)
        {
            String name = GetNameFromGfxFolder(dir.directory.relative);
            vcs->RegisterAvailableResourceSize(static_cast<int32>(currentSize.dx * dir.scale), static_cast<int32>(currentSize.dy * dir.scale), name);
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void ResourceSelectorModule::CreateAction(const QString& actionName, const QString& prevActionName, DAVA::int32 gfxMode)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<ResourceSelectorData>();
    fieldDescr.fieldName = FastName("preferredMode");

    QtAction* action = new QtAction(GetAccessor(), actionName);
    action->SetStateUpdationFunction(QtAction::Checked, fieldDescr, [gfxMode](const Any& fieldValue) -> Any {
        return fieldValue.Cast<DAVA::int32>(0) == gfxMode;
    });

    connections.AddConnection(action, &QAction::triggered, Bind(&ResourceSelectorModule::OnGfxSelected, this, gfxMode));

    QList<QString> menuResources = QList<QString>{ "View", "Resources" };
    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(menuResources, { InsertionParams::eInsertionMethod::AfterItem, prevActionName }));

    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);

    gfxActionPlacementName.emplace_back(actionName);
}

void ResourceSelectorModule::OnGraphicsSettingsChanged()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    DataContext* globalContext = GetAccessor()->GetGlobalContext();

    ResourceSelectorData* selectorData = globalContext->GetData<ResourceSelectorData>();
    DVASSERT(selectorData != nullptr);

    ProjectData* projectData = globalContext->GetData<ProjectData>();
    if (projectData != nullptr)
    {
        const Vector<ProjectData::GfxDir>& gfxDirectories = projectData->GetGfxDirectories();
        int32 gfxCount = static_cast<int32>(gfxDirectories.size());
        if (selectorData->preferredMode == gfxCount)
        {
            const EngineContext* engineContext = GetEngineContext();
            VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;

            int32 index = vcs->GetDesirableResourceIndex();
            if (resourceIndex != index)
            {
                resourceIndex = index;
                delayedExecutor.DelayedExecute(DAVA::MakeFunction(this, &ResourceSelectorModule::ReloadSpritesImpl));
            }
        }
    }
}

void ResourceSelectorModule::OnWindowResized(DAVA::Size2f windowSize)
{
    using namespace DAVA;

    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    vcs->SetVirtualScreenSize(static_cast<int32>(windowSize.dx), static_cast<int32>(windowSize.dy));

    RegisterGfxFolders();
}

void ResourceSelectorModule::ReloadSpritesImpl()
{
    DAVA::TArc::WaitDialogParams waitDlgParams;
    waitDlgParams.message = "Sprite Reloading";
    waitDlgParams.needProgressBar = false;
    std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle = GetUI()->ShowWaitDialog(DAVA::TArc::mainWindowKey, waitDlgParams);

    DAVA::Sprite::ReloadSprites();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ResourceSelectorModule)
{
    DAVA::ReflectionRegistrator<ResourceSelectorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(ResourceSelectorModule);
