#include "Modules/LibraryModule/LibraryModule.h"
#include "Modules/LibraryModule/Private/LibraryWidget.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Application/QEGlobal.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryModule)
{
    DAVA::ReflectionRegistrator<LibraryModule>::Begin()
    .ConstructorByPointer()
    .End();
}

QString LibraryModule::toolbarName = "Library Controls Toolbar";

void LibraryModule::PostInit()
{
    InitUI();
    BindFields();
    RegisterOperations();
}

void LibraryModule::InitUI()
{
    using namespace DAVA::TArc;

    QString title = "Library";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::LeftDockWidgetArea;
    PanelKey panelKey(title, panelInfo);

    libraryWidget = new LibraryWidget(GetAccessor(), GetUI());
    GetUI()->AddView(DAVA::TArc::mainWindowKey, panelKey, libraryWidget);

    ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                << "Toolbars"));
    GetUI()->DeclareToolbar(DAVA::TArc::mainWindowKey, toolbarTogglePlacement, toolbarName);

    //QToolBar* toolbar = mainWindowInfo.window->findChild<QToolBar*>(toolbarName);

    //     QToolButton* button = new QToolButton();
    //     button->setAutoRaise(true);
    //     button->setText("other");
    //     //button->setIcon(QIcon(":/TArc/Resources/openfile.png"));
    //     button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //     button->setObjectName("otherControlsToolButton");

    //ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
    //GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
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

void LibraryModule::RegisterOperations()
{
    //RegisterOperation(QEGlobal::SelectFile.ID, widget, &LibraryWidget::SelectFile);
}

void LibraryModule::OnPackageChanged(const DAVA::Any& package)
{
    PackageNode* packageNode = nullptr;
    if (package.CanGet<PackageNode*>())
    {
        packageNode = package.Get<PackageNode*>();
    }

    if (currentPackageNode != nullptr)
    {
        currentPackageNode->RemoveListener(this);
        RemovePrototypesMenus();
    }

    currentPackageNode = packageNode;
    if (currentPackageNode != nullptr)
    {
        currentPackageNode->AddListener(this);
        AddPrototypesMenus(packageNode);
    }

    libraryWidget->SetCurrentPackage(packageNode);
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

    libraryWidget->SetLibraryPackages(libraryPackages);
}

DAVA::Vector<DAVA::RefPtr<PackageNode>> LibraryModule::LoadLibraryPackages(ProjectData* projectData)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DAVA::Vector<DAVA::RefPtr<PackageNode>> libraryPackages;

    for (const ProjectData::LibrarySection& section : projectData->GetLibrarySections())
    {
        const EngineContext* engineContext = GetEngineContext();
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

void LibraryModule::AddControlsMenus(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages)
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    const Vector<ProjectData::PinnedControl>& pinnedControls = projectData->GetPinnedControls();
    for (const ProjectData::PinnedControl& pinnedControl : pinnedControls)
    {
        QString controlName = QString::fromStdString(pinnedControl.packagePath.absolute.GetBasename());
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

                ActionInfo actionInfo;
                actionInfo.action = new QtAction(accessor, QIcon(controlIconPath), controlName);
                //connections.AddConnection(action, &QAction::triggered, Bind(&LibraryModule::OnToCreate, this));

                //         FieldDescriptor fieldDescr;
                //         fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
                //         fieldDescr.fieldName = FastName(DocumentData::canSavePropertyName);
                //         action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
                //             return fieldValue.Cast<bool>(false);
                //         });

                actionInfo.placement.AddPlacementPoint(CreateMenuPoint("Controls"));
                actionInfo.placement.AddPlacementPoint(CreateToolbarPoint(toolbarName));
                ui->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);

                controlsActions.emplace(controlNode, std::move(actionInfo));
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
        QString sectionName = QString::fromStdString(section.packagePath.absolute.GetBasename());
        QString sectionIconPath = QString::fromStdString(section.iconPath.absolute.GetAbsolutePathname());
        QUrl menuPoint = section.pinned ? CreateMenuPoint(QList<QString>() << "Controls" << sectionName) : CreateMenuPoint(QList<QString>() << "Controls"
                                                                                                                                            << "Other" << sectionName);

        auto pkgFound = std::find_if(libraryPackages.begin(), libraryPackages.end(), [&section](const RefPtr<PackageNode>& pkg)
                                     {
                                         return (pkg->GetPath() == section.packagePath.absolute);
                                     });
        if (pkgFound != libraryPackages.end())
        {
            PackageControlsNode* packageControls = pkgFound->Get()->GetPackageControlsNode();
            for (ControlNode* node : *packageControls)
            {
                QString className = QString::fromStdString(node->GetControl()->GetClassName());
                QString iconPath = IconHelper::GetIconPathForClassName(className);
                QString controlName = QString::fromStdString(node->GetName());

                ActionInfo actionInfo;
                actionInfo.action = new QtAction(accessor, QIcon(iconPath), controlName);
                //connections.AddConnection(action, &QAction::triggered, Bind(&LibraryModule::OnToCreate, this));

                //         FieldDescriptor fieldDescr;
                //         fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
                //         fieldDescr.fieldName = FastName(DocumentData::canSavePropertyName);
                //         action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
                //             return fieldValue.Cast<bool>(false);
                //         });

                actionInfo.placement.AddPlacementPoint(menuPoint);
                //placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, "project actions separator" }));

                ui->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);
                controlsActions.emplace(node, std::move(actionInfo));
            }
        }
    }
}

void LibraryModule::RemoveControlsMenus()
{
    ClearActions(controlsActions);
}

void LibraryModule::AddControlAction(ControlNode* controlNode, const QUrl& menuPoint, LibraryModule::ActionsMap& actionsMap)
{
    QString iconPath = IconHelper::GetCustomIconPath();
    QString controlName = QString::fromStdString(controlNode->GetName());

    ActionInfo actionInfo;
    actionInfo.action = new DAVA::TArc::QtAction(GetAccessor(), QIcon(iconPath), controlName);
    actionInfo.placement.AddPlacementPoint(menuPoint);
    //connections.AddConnection(action, &QAction::triggered, Bind(&LibraryModule::OnToCreate, this));

    //         FieldDescriptor fieldDescr;
    //         fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    //         fieldDescr.fieldName = FastName(DocumentData::canSavePropertyName);
    //         action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
    //             return fieldValue.Cast<bool>(false);
    //         });

    //placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, "project actions separator" }));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action);
    actionsMap.emplace(controlNode, std::move(actionInfo));
}

void LibraryModule::AddPackageControlsActions(PackageControlsNode* controls, const QUrl& menuPoint, LibraryModule::ActionsMap& actionsMap)
{
    if (controls != nullptr)
    {
        for (ControlNode* prototypeNode : *controls)
        {
            AddControlAction(prototypeNode, menuPoint, actionsMap);
        }
    }
};

void LibraryModule::AddPrototypesMenus(PackageNode* packageNode)
{
    QUrl menuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << "Controls"
                                                                  << "Prototypes");
    PackageControlsNode* prototypes = packageNode->GetPrototypes();
    AddPackageControlsActions(prototypes, menuPoint, prototypesActions);

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
    PackageControlsNode* prototypes = package->GetPrototypes();
    QUrl menuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << "Controls"
                                                                  << "Prototypes"
                                                                  << "Imported Prototypes" << QString::fromStdString(package->GetName()));
    AddPackageControlsActions(prototypes, menuPoint, prototypesActions);
}

void LibraryModule::RemoveImportedPackageControlsActions(const PackageNode* package)
{
    PackageControlsNode* prototypes = package->GetPrototypes();
    for (ControlNode* controlNode : *prototypes)
    {
        RemoveControlAction(controlNode, prototypesActions);
    }
}

void LibraryModule::RemovePrototypesMenus()
{
    ClearActions(prototypesActions);
}

void LibraryModule::ClearActions(ActionsMap& actionsMap)
{
    for (const std::pair<ControlNode*, ActionInfo>& entry : actionsMap)
    {
        GetUI()->RemoveAction(DAVA::TArc::mainWindowKey, entry.second.placement, entry.second.action->text());
    }
    actionsMap.clear();
}

void LibraryModule::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        auto nodeFound = prototypesActions.find(node);
        if (nodeFound != prototypesActions.end())
        {
            nodeFound->second.action->setText(QString::fromStdString(property->GetValue().Get<String>()));
        }
    }
}

void LibraryModule::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    Q_UNUSED(destination);
    Q_UNUSED(row);
    DVASSERT(nullptr != node);

    if (node->GetParent() == currentPackageNode->GetPrototypes())
    {
        QUrl menuPoint = DAVA::TArc::CreateMenuPoint(QList<QString>() << "Controls"
                                                                      << "Prototypes");
        AddControlAction(node, menuPoint, prototypesActions);
    }
}

void LibraryModule::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);
    RemoveControlAction(node, prototypesActions);
}

void LibraryModule::RemoveControlAction(ControlNode* node, ActionsMap& actionsMap)
{
    auto nodeFound = actionsMap.find(node);
    if (nodeFound != actionsMap.end())
    {
        ActionInfo& actionInfo = nodeFound->second;
        GetUI()->RemoveAction(DAVA::TArc::mainWindowKey, actionInfo.placement, actionInfo.action->text());
        actionsMap.erase(nodeFound);
    }
}

void LibraryModule::ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    Q_UNUSED(to);
    Q_UNUSED(index);
    DVASSERT(nullptr != node);
    if (node->GetParent() == currentPackageNode->GetImportedPackagesNode())
    {
        AddImportedPackageControlsActions(node);
    }
}

void LibraryModule::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);

    if (node->GetParent() == currentPackageNode->GetImportedPackagesNode())
    {
        RemoveImportedPackageControlsActions(node);
    }
}

DECL_GUI_MODULE(LibraryModule);
