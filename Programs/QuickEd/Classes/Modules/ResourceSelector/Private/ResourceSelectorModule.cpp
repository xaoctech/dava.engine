#include "Modules/ResourceSelector/ResourceSelectorModule.h"

#include "Modules/ProjectModule/ProjectData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/SettingsNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Engine/EngineContext.h>
#include <UI/UIControlSystem.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Utils/Random.h>

namespace ResourceSelectorModuleDetails
{
class ResourceSelectorData : public DAVA::TArc::SettingsNode
{
public:
    DAVA::int32 preferredMode = 0;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ResourceSelectorData, DAVA::TArc::SettingsNode)
    {
        DAVA::ReflectionRegistrator<ResourceSelectorData>::Begin()
        .ConstructorByPointer()
        .Field("preferredMode", &ResourceSelectorData::preferredMode)[M::HiddenField()]
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

    primaryWindow->sizeChanged.Connect([this](Window* /*w*/, Size2f /* windowSize*/, Size2f /* surfaceSize */)
                                       {
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
    if (projectData == nullptr)
    { //project was closed
        for (ActionDescriptor& descriptor : gfxActions)
        {
            ui->RemoveAction(descriptor.windowKey, descriptor.placement);
        }

        gfxActions.clear();
    }
    else
    { //Project Opened

        const Vector<ProjectData::GfxDir>& gfxDirectories = projectData->GetGfxDirectories();
        if (gfxDirectories.empty() == false)
        {
            QString prevActionName = "";

            int32 count = static_cast<int32>(gfxDirectories.size());
            for (int32 ia = 0; ia < count; ++ia)
            {
                const ProjectData::GfxDir& gfx = gfxDirectories[ia];
                QString actionName = QString::fromStdString(GetNameFromGfxFolder(gfx.directory.relative));
                CreateAction(actionName, prevActionName, ia);
                prevActionName = actionName;
            }

            CreateAction("Auto", prevActionName, count);

            ResourceSelectorData* selectorData = globalContext->GetData<ResourceSelectorData>();
            DVASSERT(selectorData != nullptr);
            OnGfxSelected(selectorData->preferredMode);
        }
    }
}

void ResourceSelectorModule::OnGfxSelected(DAVA::int32 gfxMode)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace ResourceSelectorModuleDetails;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    ResourceSelectorData* selectorData = globalContext->GetData<ResourceSelectorData>();
    DVASSERT(selectorData != nullptr);
    selectorData->preferredMode = gfxMode;

    ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);

    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    Size2i currentSize = vcs->GetVirtualScreenSize();

    const Vector<ProjectData::GfxDir>& gfxDirectories = projectData->GetGfxDirectories();
    int32 gfxCount = static_cast<int32>(gfxDirectories.size());
    if (gfxMode < gfxCount)
    {
        vcs->UnregisterAllAvailableResourceSizes();

        String name = GetNameFromGfxFolder(gfxDirectories[gfxMode].directory.relative);
        vcs->RegisterAvailableResourceSize(currentSize.dx, currentSize.dy, name);

        Sprite::ReloadSprites();
    }
    else if (gfxMode == gfxCount)
    {
        vcs->UnregisterAllAvailableResourceSizes();

        for (const ProjectData::GfxDir& dir : gfxDirectories)
        {
            String name = GetNameFromGfxFolder(dir.directory.relative);
            vcs->RegisterAvailableResourceSize(static_cast<int32>(currentSize.dx * dir.scale), static_cast<int32>(currentSize.dy * dir.scale), name);
        }

        Sprite::ReloadSprites();
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

    gfxActions.emplace_back(DAVA::TArc::mainWindowKey, placementInfo);
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
    DVASSERT(projectData != nullptr);
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
            Sprite::ReloadSprites();
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ResourceSelectorModule)
{
    DAVA::ReflectionRegistrator<ResourceSelectorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(ResourceSelectorModule);
