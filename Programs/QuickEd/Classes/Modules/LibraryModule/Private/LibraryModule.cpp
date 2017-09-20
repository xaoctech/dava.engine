#include "Modules/LibraryModule/LibraryModule.h"
#include "Modules/LibraryModule/Private/LibraryWidget.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Application/QEGlobal.h"

#include <QToolButton>
#include <QMenu>

#include <TArc/Core/FieldBinder.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

namespace LibraryModuleDetails
{
QString CreateMenuName(QString name)
{
#if defined __DAVAENGINE_MACOS__
    // toolbar buttons with menu do look differently on Mac and Win. On Mac, button text overlaps with dropdown arrow symbol.
    // Adding spaces should fix this overlap issue.
    return name + QStringLiteral("  ");
#else
    return name;
#endif
}

const QString controlsToolbarName = "Library Controls Toolbar";
const QString menuNameLibrary = CreateMenuName("Library");
const QString menuNamePrototypes = CreateMenuName("Prototypes");
};

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryModule)
{
    DAVA::ReflectionRegistrator<LibraryModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void LibraryModule::PostInit()
{
    InitData();
    InitUI();
    BindFields();
}

void LibraryModule::InitData()
{
    std::unique_ptr<LibraryData> data = std::make_unique<LibraryData>();
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void LibraryModule::InitUI()
{
    using namespace DAVA::TArc;
    using namespace LibraryModuleDetails;

    { // create Library dock panel
        QString dockPanelTitle = "Library";
        DockPanelInfo dockPanelInfo;
        dockPanelInfo.title = dockPanelTitle;
        dockPanelInfo.area = Qt::LeftDockWidgetArea;
        PanelKey dockPanelKey(dockPanelTitle, dockPanelInfo);

        GetLibraryData()->libraryWidget = new LibraryWidget(GetAccessor(), GetUI());
        GetUI()->AddView(DAVA::TArc::mainWindowKey, dockPanelKey, GetLibraryData()->libraryWidget);
    }

    { // create toolbar
        ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                    << "Toolbars"));
        GetUI()->DeclareToolbar(DAVA::TArc::mainWindowKey, toolbarTogglePlacement, LibraryModuleDetails::controlsToolbarName);
    }

    { // create menu "Controls", insert before "Help"
        QMenu* controlsMenu = new QMenu(QStringLiteral("Controls"), nullptr);
        ActionPlacementInfo controlsMenuPlacement(CreateMenuPoint("", { DAVA::TArc::InsertionParams::eInsertionMethod::BeforeItem, MenuItems::menuHelp }));
        GetUI()->AddAction(mainWindowKey, controlsMenuPlacement, controlsMenu->menuAction());
    }

    { // create menu "Library", insert into Controls menu and Controls toolbar
        QAction* libraryMenu = new QAction(menuNameLibrary, nullptr);
        libraryMenu->setObjectName(menuNameLibrary);
        ActionPlacementInfo placement;
        placement.AddPlacementPoint(CreateMenuPoint("Controls"));
        placement.AddPlacementPoint(CreateToolbarPoint(controlsToolbarName));
        GetUI()->AddAction(mainWindowKey, placement, libraryMenu);
    }
}

void LibraryModule::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(GetAccessor()));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &LibraryModule::OnProjectPathChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &LibraryModule::OnPackageChanged));
    }
}

void LibraryModule::OnPackageChanged(const DAVA::Any& package)
{
    PackageNode* packageNode = nullptr;
    if (package.CanGet<PackageNode*>())
    {
        packageNode = package.Get<PackageNode*>();
    }

    LibraryData* data = GetLibraryData();

    if (data->currentPackageNode != nullptr)
    {
        data->currentPackageNode->RemoveListener(this);
        RemovePrototypesMenus();
    }

    data->currentPackageNode = packageNode;
    if (data->currentPackageNode != nullptr)
    {
        data->currentPackageNode->AddListener(this);
        AddPrototypesMenus(packageNode);
    }

    data->libraryWidget->SetCurrentPackage(packageNode);
}

void LibraryModule::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    RemoveControlsMenus();

    Vector<RefPtr<PackageNode>> libraryPackages;

    if (projectPath.Cast<FilePath>(FilePath()).IsEmpty() == false)
    {
        const DataContext* globalContext = GetAccessor()->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        DVASSERT(projectData != nullptr);
        libraryPackages = LoadLibraryPackages(projectData);

        AddControlsMenus(projectData, libraryPackages);
    }

    GetLibraryData()->libraryWidget->SetLibraryPackages(libraryPackages);
}

void LibraryModule::OnControlCreateTriggered(ControlNode* node)
{
    InvokeOperation(QEGlobal::CreateByClick.ID, node);
}

DAVA::Vector<DAVA::RefPtr<PackageNode>> LibraryModule::LoadLibraryPackages(ProjectData* projectData)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    const EngineContext* engineContext = GetEngineContext();

    DAVA::Vector<DAVA::RefPtr<PackageNode>> libraryPackages;

    for (const ProjectData::LibrarySection& section : projectData->GetLibrarySections())
    {
        QuickEdPackageBuilder builder(engineContext);
        PackageNode* package = nullptr;
        if (UIPackageLoader(projectData->GetPrototypes()).LoadPackage(section.packagePath.absolute, &builder))
        {
            RefPtr<PackageNode> libraryPackage = builder.BuildPackage();
            if (builder.GetResults().HasErrors())
            {
                NotificationParams params;
                params.title = "Can't load library package";
                params.message.type = Result::RESULT_ERROR;
                params.message.message = Format("Package '%s' has problems...", section.packagePath.absolute.GetFilename().c_str());
                GetUI()->ShowNotification(DAVA::TArc::mainWindowKey, params);
            }
            else
            {
                libraryPackages.emplace_back(libraryPackage);
            }
        }
    }

    return libraryPackages;
}

LibraryData* LibraryModule::GetLibraryData()
{
    return GetAccessor()->GetGlobalContext()->GetData<LibraryData>();
}

void LibraryModule::AddControlsMenus(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace LibraryModuleDetails;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    const Vector<ProjectData::PinnedControl>& pinnedControls = projectData->GetPinnedControls();
    for (const ProjectData::PinnedControl& pinnedControl : pinnedControls)
    {
        auto pkgFound = std::find_if(libraryPackages.begin(), libraryPackages.end(), [&pinnedControl](const RefPtr<PackageNode>& pkg)
                                     {
                                         return (pkg->GetPath() == pinnedControl.packagePath.absolute);
                                     });
        if (pkgFound != libraryPackages.end())
        {
            PackageControlsNode* packageControls = pkgFound->Get()->GetPackageControlsNode();
            ControlNode* controlNode = packageControls->FindControlNodeByName(pinnedControl.controlName);
            if (controlNode != nullptr)
            {
                QString controlIconPath;
                if (pinnedControl.iconPath.absolute.IsEmpty())
                {
                    QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
                    controlIconPath = IconHelper::GetIconPathForClassName(className);
                }
                else
                {
                    controlIconPath = QString::fromStdString(pinnedControl.iconPath.absolute.GetAbsolutePathname());
                }

                LibraryData::ActionInfo actionInfo;
                actionInfo.action = new QtAction(accessor, QIcon(controlIconPath), QString::fromStdString(pinnedControl.controlName));
                InsertionParams insertionParams = { InsertionParams::eInsertionMethod::BeforeItem, menuNameLibrary };
                actionInfo.placement.AddPlacementPoint(CreateMenuPoint("Controls", insertionParams));
                actionInfo.placement.AddPlacementPoint(CreateToolbarPoint(LibraryModuleDetails::controlsToolbarName, insertionParams));
                connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, controlNode));
                ui->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);

                GetLibraryData()->controlsActions.emplace(controlNode, std::move(actionInfo));
            }
            else
            {
                NotificationParams params;
                params.title = "Project file contains errors";
                params.message.type = Result::RESULT_ERROR;
                params.message.message = Format("Can't find pinned control '%s' described in Control section", pinnedControl.controlName.c_str());
                GetUI()->ShowNotification(DAVA::TArc::mainWindowKey, params);
            }
        }
    }

    const Vector<ProjectData::LibrarySection> librarySections = projectData->GetLibrarySections();
    for (const ProjectData::LibrarySection& section : librarySections)
    {
        auto pkgFound = std::find_if(libraryPackages.begin(), libraryPackages.end(), [&section](const RefPtr<PackageNode>& pkg)
                                     {
                                         return (pkg->GetPath() == section.packagePath.absolute);
                                     });
        if (pkgFound != libraryPackages.end())
        {
            QString sectionName = CreateMenuName(QString::fromStdString(section.packagePath.absolute.GetBasename()));
            QString sectionIconPath = QString::fromStdString(section.iconPath.absolute.GetAbsolutePathname());

            QUrl menuPoint = CreateMenuPoint(QList<QString>() << "Controls" << menuNameLibrary << sectionName);
            QUrl toolbarMenuPoint = CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNameLibrary << sectionName);

            QUrl pinnedMenuPoint;
            QUrl pinnedToolbarMenuPoint;

            if (section.pinned == true)
            {
                { // create menu, insert into Controls menu and Controls toolbar
                    QAction* sectionMenu = new QAction(sectionName, nullptr);
                    ActionPlacementInfo placement;
                    InsertionParams insertionParams = { DAVA::TArc::InsertionParams::eInsertionMethod::BeforeItem, menuNameLibrary };
                    placement.AddPlacementPoint(CreateMenuPoint("Controls", insertionParams));
                    placement.AddPlacementPoint(CreateToolbarPoint(controlsToolbarName, insertionParams));
                    GetUI()->AddAction(mainWindowKey, placement, sectionMenu);
                }

                pinnedMenuPoint = CreateMenuPoint(QList<QString>() << "Controls" << sectionName);
                pinnedToolbarMenuPoint = CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << sectionName);
            }

            PackageControlsNode* packageControls = pkgFound->Get()->GetPackageControlsNode();
            for (ControlNode* node : *packageControls)
            {
                QString className = QString::fromStdString(node->GetControl()->GetClassName());
                QString iconPath = IconHelper::GetIconPathForClassName(className);
                QString controlName = QString::fromStdString(node->GetName());

                LibraryData::ActionInfo actionInfo;
                actionInfo.action = new QtAction(accessor, QIcon(iconPath), controlName);
                actionInfo.placement.AddPlacementPoint(menuPoint);
                actionInfo.placement.AddPlacementPoint(toolbarMenuPoint);
                if (section.pinned == true)
                {
                    actionInfo.placement.AddPlacementPoint(pinnedMenuPoint);
                    actionInfo.placement.AddPlacementPoint(pinnedToolbarMenuPoint);
                }
                connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, node));

                ui->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);
                GetLibraryData()->controlsActions.emplace(node, std::move(actionInfo));
            }
        }
    }
}

void LibraryModule::RemoveControlsMenus()
{
    ClearActions(GetLibraryData()->controlsActions);
}

void LibraryModule::AddControlAction(ControlNode* controlNode, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, LibraryData::ActionsMap& actionsMap)
{
    QString iconPath = IconHelper::GetCustomIconPath();
    QString controlName = QString::fromStdString(controlNode->GetName());

    LibraryData::ActionInfo actionInfo;
    actionInfo.action = new DAVA::TArc::QtAction(GetAccessor(), QIcon(iconPath), controlName);
    actionInfo.placement.AddPlacementPoint(menuPoint);
    actionInfo.placement.AddPlacementPoint(toolbarMenuPoint);
    connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, controlNode));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);
    actionsMap.emplace(controlNode, std::move(actionInfo));
}

void LibraryModule::AddPackageControlsActions(PackageControlsNode* controls, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, LibraryData::ActionsMap& actionsMap)
{
    if (controls != nullptr)
    {
        for (ControlNode* prototypeNode : *controls)
        {
            AddControlAction(prototypeNode, menuPoint, toolbarMenuPoint, actionsMap);
        }
    }
};

void LibraryModule::AddPrototypesMenus(PackageNode* packageNode)
{
    using namespace LibraryModuleDetails;

    QUrl menuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << "Controls" << menuNamePrototypes);
    QUrl toolbarMenuPoint = DAVA::TArc::CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNamePrototypes);

    PackageControlsNode* prototypes = packageNode->GetPrototypes();
    AddPackageControlsActions(prototypes, menuPoint, toolbarMenuPoint, GetLibraryData()->prototypesActions);

    ImportedPackagesNode* importedPackages = packageNode->GetImportedPackagesNode();
    if (importedPackages != nullptr)
    {
        for (const PackageNode* package : *importedPackages)
        {
            AddImportedPackageControlsActions(package);
        }
    }
}

void LibraryModule::AddImportedPackageControlsActions(const PackageNode* package)
{
    using namespace LibraryModuleDetails;

    QList<QString> path({ "Controls", menuNamePrototypes, QString::fromStdString(package->GetName()) });
    QUrl menuPoint = DAVA::TArc::CreateMenuPoint(path);

    path.pop_front();
    QUrl toolbarMenuPoint = DAVA::TArc::CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, path);

    PackageControlsNode* prototypes = package->GetPrototypes();
    AddPackageControlsActions(prototypes, menuPoint, toolbarMenuPoint, GetLibraryData()->prototypesActions);
}

void LibraryModule::RemoveImportedPackageControlsActions(const PackageNode* package)
{
    PackageControlsNode* prototypes = package->GetPrototypes();
    for (ControlNode* controlNode : *prototypes)
    {
        RemoveControlAction(controlNode, GetLibraryData()->prototypesActions);
    }
}

void LibraryModule::RemovePrototypesMenus()
{
    ClearActions(GetLibraryData()->prototypesActions);
}

void LibraryModule::ClearActions(LibraryData::ActionsMap& actionsMap)
{
    for (const std::pair<ControlNode*, LibraryData::ActionInfo>& entry : actionsMap)
    {
        GetUI()->RemoveAction(DAVA::TArc::mainWindowKey, entry.second.placement, entry.second.action->text());
    }
    actionsMap.clear();
}

void LibraryModule::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        LibraryData* data = GetLibraryData();
        auto nodeFound = data->prototypesActions.find(node);
        if (nodeFound != data->prototypesActions.end())
        {
            nodeFound->second.action->setText(QString::fromStdString(property->GetValue().Get<String>()));
        }
    }
}

void LibraryModule::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    using namespace LibraryModuleDetails;

    Q_UNUSED(destination);
    Q_UNUSED(row);
    DVASSERT(nullptr != node);

    LibraryData* data = GetLibraryData();

    if (node->GetParent() == data->currentPackageNode->GetPrototypes())
    {
        QUrl menuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << "Controls" << menuNamePrototypes);
        QUrl toolbarMenuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << menuNamePrototypes);
        AddControlAction(node, menuPoint, toolbarMenuPoint, data->prototypesActions);
    }
}

void LibraryModule::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);
    RemoveControlAction(node, GetLibraryData()->prototypesActions);
}

void LibraryModule::RemoveControlAction(ControlNode* node, LibraryData::ActionsMap& actionsMap)
{
    auto nodeFound = actionsMap.find(node);
    if (nodeFound != actionsMap.end())
    {
        LibraryData::ActionInfo& actionInfo = nodeFound->second;
        GetUI()->RemoveAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action->text());
        actionsMap.erase(nodeFound);
    }
}

void LibraryModule::ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    Q_UNUSED(to);
    Q_UNUSED(index);
    DVASSERT(nullptr != node);
    if (node->GetParent() == GetLibraryData()->currentPackageNode->GetImportedPackagesNode())
    {
        AddImportedPackageControlsActions(node);
    }
}

void LibraryModule::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);

    if (node->GetParent() == GetLibraryData()->currentPackageNode->GetImportedPackagesNode())
    {
        RemoveImportedPackageControlsActions(node);
    }
}

DECL_GUI_MODULE(LibraryModule);
