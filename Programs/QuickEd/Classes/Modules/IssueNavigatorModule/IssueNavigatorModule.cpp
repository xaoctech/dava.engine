#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorModule.h"

#include "Classes/Modules/IssueNavigatorModule/IssueData.h"
#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorData.h"
#include "Classes/Modules/IssueNavigatorModule/LayoutIssueHandler.h"
#include "Classes/Modules/IssueNavigatorModule/NamingIssuesHandler.h"
#include "Classes/Modules/IssueNavigatorModule/EventsIssuesHandler.h"

#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/TableView.h>

DAVA_VIRTUAL_REFLECTION_IMPL(IssueNavigatorModule)
{
    DAVA::ReflectionRegistrator<IssueNavigatorModule>::Begin()
    .ConstructorByPointer()
    .Field("header", &IssueNavigatorModule::headerDescription)
    .Field("issues", &IssueNavigatorModule::GetValues, nullptr)
    .Field("current", &IssueNavigatorModule::GetCurrentValue, &IssueNavigatorModule::SetCurrentValue)
    .Method("OnIssueActivated", &IssueNavigatorModule::OnIssueAvitvated)
    .End();
}

DAVA_REFLECTION_IMPL(IssueNavigatorModule::HeaderDescription)
{
    DAVA::ReflectionRegistrator<IssueNavigatorModule::HeaderDescription>::Begin()
    .Field("message", &IssueNavigatorModule::HeaderDescription::message)[DAVA::M::DisplayName("Message")]
    .Field("pathToControl", &IssueNavigatorModule::HeaderDescription::pathToControl)[DAVA::M::DisplayName("Path To Control")]
    .Field("packagePath", &IssueNavigatorModule::HeaderDescription::packagePath)[DAVA::M::DisplayName("Package Path")]
    .Field("propertyName", &IssueNavigatorModule::HeaderDescription::propertyName)[DAVA::M::DisplayName("Property Name")]
    .End();
}

void IssueNavigatorModule::PostInit()
{
    using namespace DAVA::TArc;
    const char* title = "Issue Navigator";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;

    PanelKey key(title, panelInfo);
    TableView::Params params(GetAccessor(), GetUI(), DAVA::TArc::mainWindowKey);
    params.fields[TableView::Fields::Header] = "header";
    params.fields[TableView::Fields::Values] = "issues";
    params.fields[TableView::Fields::CurrentValue] = "current";
    params.fields[TableView::Fields::ItemActivated] = "OnIssueActivated";

    TableView* tableView = new TableView(params, GetAccessor(), DAVA::Reflection::Create(DAVA::ReflectedObject(this)));
    GetUI()->AddView(DAVA::TArc::mainWindowKey, key, tableView->ToWidgetCast());

    std::unique_ptr<IssueNavigatorData> data = std::make_unique<IssueNavigatorData>();
    DAVA::int32 sectionId = 0;
    data->handlers.push_back(std::make_unique<LayoutIssueHandler>(GetAccessor(), GetUI(), sectionId++, &data->indexGenerator));
    data->handlers.push_back(std::make_unique<NamingIssuesHandler>(GetAccessor(), GetUI(), sectionId++, &data->indexGenerator));
    data->handlers.push_back(std::make_unique<EventsIssuesHandler>(GetAccessor(), GetUI(), sectionId++, &data->indexGenerator));

    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void IssueNavigatorModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    if (key == DAVA::TArc::mainWindowKey)
    {
        GetAccessor()->GetGlobalContext()->DeleteData<IssueNavigatorData>();
    }
}

void IssueNavigatorModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    context->CreateData(std::make_unique<IssueData>());
}

void IssueNavigatorModule::OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne)
{
    if (current != nullptr)
    {
        IssueData* issueData = current->GetData<IssueData>();
        issueData->RemoveAllIssues();
    }
}

void IssueNavigatorModule::OnIssueAvitvated(DAVA::int32 index)
{
    const DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    if (context)
    {
        DVASSERT(index != -1);

        IssueData* issueData = context->GetData<IssueData>();
        if (issueData)
        {
            const IssueData::Issue& issue = issueData->GetIssues()[index];
            const QString& path = QString::fromStdString(DAVA::FilePath(issue.packagePath).GetAbsolutePathname());
            const QString& name = QString::fromStdString(issue.pathToControl);
            InvokeOperation(QEGlobal::SelectControl.ID, path, name);
        }
    }
}

const DAVA::Vector<IssueData::Issue>& IssueNavigatorModule::GetValues() const
{
    const DAVA::TArc::DataContext* context = GetAccessor()->GetActiveContext();
    if (context)
    {
        IssueData* issueData = context->GetData<IssueData>();
        DVASSERT(issueData);
        return issueData->GetIssues();
    }

    static DAVA::Vector<IssueData::Issue> empty;
    return empty;
}

void IssueNavigatorModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    const DAVA::TArc::DataContext* globalContext = GetAccessor()->GetGlobalContext();
    if (globalContext)
    {
        IssueNavigatorData* data = globalContext->GetData<IssueNavigatorData>();
        DVASSERT(data);
        for (const std::unique_ptr<IssueHandler>& handler : data->handlers)
        {
            handler->OnContextDeleted(context);
        }
    }
}

DAVA::Any IssueNavigatorModule::GetCurrentValue() const
{
    return current;
}

void IssueNavigatorModule::SetCurrentValue(const DAVA::Any& currentValue)
{
    if (currentValue.IsEmpty())
    {
        current = 0;
    }
    else
    {
        current = currentValue.Cast<DAVA::int32>();
    }
}

DECL_GUI_MODULE(IssueNavigatorModule);
