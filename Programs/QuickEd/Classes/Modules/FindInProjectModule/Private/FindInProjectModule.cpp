#include "Modules/FindInProjectModule/FindInProjectModule.h"
#include "Application/QEGlobal.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "UI/Find/Filters/FindFilter.h"
#include "UI/Find/Widgets/FindInProjectDialog.h"
#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/Find/FindInDocumentController.h"

#include <TArc/DataProcessing/Common.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

namespace FindInProjectDetail
{
class FindInProjectData : public DAVA::TArcDataNode
{
public:
    std::unique_ptr<FindInDocumentController> widgetController;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(FindInProjectData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<FindInProjectData>::Begin()
        .End();
    }
};
}

DAVA_VIRTUAL_REFLECTION_IMPL(FindInProjectModule)
{
    DAVA::ReflectionRegistrator<FindInProjectModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindInProjectModule::PostInit()
{
    using namespace DAVA;

    UI* ui = GetUI();
    ContextAccessor* accessor = GetAccessor();

    FieldDescriptor packageFieldDescr;
    packageFieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
    packageFieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);

    const auto updater =
    [](const Any& fieldValue) -> Any {
        return !fieldValue.Cast<FilePath>(FilePath()).IsEmpty();
    };

    QtAction* findInProjectAction = new QtAction(accessor, QObject::tr("Find in Project..."), nullptr);
    findInProjectAction->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_F);
    findInProjectAction->SetStateUpdationFunction(QtAction::Enabled, packageFieldDescr, updater);

    connections.AddConnection(findInProjectAction, &QAction::triggered, MakeFunction(this, &FindInProjectModule::OnFindInProject));

    ActionPlacementInfo placementInfo(CreateMenuPoint("Find", InsertionParams(InsertionParams::eInsertionMethod::BeforeItem, "Select Current Document in File System")));
    ui->AddAction(DAVA::mainWindowKey, placementInfo, findInProjectAction);

    FindInProjectDetail::FindInProjectData* data = new FindInProjectDetail::FindInProjectData();
    data->widgetController.reset(new FindInDocumentController(ui, accessor));
    connections.AddConnection(data->widgetController.get(), &FindInDocumentController::FindInDocumentRequest, [this](std::shared_ptr<FindFilter> filter)
                              {
                                  InvokeOperation(QEGlobal::FindInDocument.ID, filter);
                              });

    connections.AddConnection(data->widgetController.get(), &FindInDocumentController::SelectControlRequest, [this](const QString& path, const QString& name)
                              {
                                  InvokeOperation(QEGlobal::SelectControl.ID, path, name);
                              });

    GetAccessor()->GetGlobalContext()->CreateData(std::unique_ptr<DAVA::TArcDataNode>(data));
}

void FindInProjectModule::OnFindInProject()
{
    FindInProjectDialog findInProjectDialog;
    if (findInProjectDialog.exec() == QDialog::Accepted)
    {
        InvokeOperation(QEGlobal::FindInProject.ID, std::shared_ptr<FindFilter>(findInProjectDialog.BuildFindFilter()));
    }
}

DECL_GUI_MODULE(FindInProjectModule);
