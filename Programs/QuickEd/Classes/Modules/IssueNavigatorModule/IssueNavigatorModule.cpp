#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorModule.h"

#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorWidget.h"
#include "Classes/Modules/IssueNavigatorModule/LayoutIssuesHandler.h"
#include "Classes/Modules/IssueNavigatorModule/NamingIssuesHandler.h"
#include "Classes/Modules/IssueNavigatorModule/EventsIssuesHandler.h"

#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/QtBoxLayouts.h>

DAVA_VIRTUAL_REFLECTION_IMPL(IssueNavigatorModule)
{
    DAVA::ReflectionRegistrator<IssueNavigatorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void IssueNavigatorModule::PostInit()
{
    using namespace DAVA;
    const char* title = "Issue Navigator";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    PanelKey key(title, panelInfo);

    widget = new IssueNavigatorWidget(GetAccessor());
    connections.AddConnection(widget, &IssueNavigatorWidget::JumpToControl, MakeFunction(this, &IssueNavigatorModule::JumpToControl));
    connections.AddConnection(widget, &IssueNavigatorWidget::JumpToPackage, MakeFunction(this, &IssueNavigatorModule::JumpToPackage));

    GetUI()->AddView(DAVA::mainWindowKey, key, widget);

    DAVA::int32 sectionId = 0;
    issuesHandlers.emplace_back(new LayoutIssuesHandler(GetAccessor(), GetUI(), sectionId++, widget, indexGenerator));
    issuesHandlers.emplace_back(new NamingIssuesHandler(GetAccessor(), GetUI(), sectionId++, widget, indexGenerator));
    issuesHandlers.emplace_back(new EventsIssuesHandler(GetAccessor(), GetUI(), sectionId++, widget, indexGenerator));
}

void IssueNavigatorModule::OnContextWasChanged(DAVA::DataContext* current, DAVA::DataContext* oldOne)
{
    for (const std::unique_ptr<IssuesHandler>& handler : issuesHandlers)
    {
        handler->OnContextActivated(current);
    }
}

void IssueNavigatorModule::OnContextDeleted(DAVA::DataContext* context)
{
    for (const std::unique_ptr<IssuesHandler>& handler : issuesHandlers)
    {
        handler->OnContextDeleted(context);
    }
}

void IssueNavigatorModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    const QString& name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void IssueNavigatorModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

DECL_TARC_MODULE(IssueNavigatorModule);
