#include "VisualScriptEditor/VisualScriptEditorDialog.h"
#include "VisualScriptEditor/Private/VisualScriptEditorPropertiesView.h"

#include "VisualScriptEditor/Private/VisualScriptEditorDialogSettings.h"
#include "VisualScriptEditor/Private/VisualScriptEditorData.h"
#include "VisualScriptEditor/Private/Models/VisualScriptRegistryModel.h"
#include "VisualScriptEditor/Private/Models/VisualScriptNodeModel.h"
#include "VisualScriptEditor/Private/Models/VisualScriptFlowScene.h"
#include "VisualScriptEditor/Private/Models/VisualScriptFlowView.h"

#include <nodes/ConnectionStyle>
#include <nodes/Node>
#include <nodes/NodeDataModel>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/PropertyPanel/TimerUpdater.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Debug/DVAssert.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedType.h>

#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptExecutor.h>
#include <VisualScript/VisualScriptNode.h>
#include <VisualScript/VisualScriptPin.h>
#include <VisualScript/Nodes/VisualScriptEventNode.h>
#include <VisualScript/Nodes/VisualScriptAnotherScriptNode.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QTabWidget>
#include <QWidget>
#include <QAction>

#include <iterator>

namespace DAVA
{
namespace VisualScriptEditorDialogDetails
{
void SetStyle()
{
    QtNodes::ConnectionStyle::setConnectionStyle(
    R"({
         "ConnectionStyle": {
             "ConstructionColor": "gray",
             "NormalColor": "black",
             "SelectedColor": "gray",
             "SelectedHaloColor": "deepskyblue",
             "HoveredColor": "deepskyblue",
             
             "LineWidth": 3.0,
             "ConstructionLineWidth": 2.0,
             "PointDiameter": 10.0,
             
             "UseDataDefinedColors": true
         }
     })");
}

void DeleteScript(ScriptDescriptor& descriptor)
{
    descriptor.reflectionHolder->reflectedModels.clear();
    descriptor.flowScene->clearScene();

    delete descriptor.flowView;
    delete descriptor.flowScene;
    delete descriptor.reflectionHolder;

    if (descriptor.isScriptOwner == true)
    {
        delete descriptor.script;
        descriptor.isScriptOwner = false;
    }
    descriptor.script = nullptr;
}
} //VisualScriptEditorDialogDetails

VisualScriptEditorDialog::VisualScriptEditorDialog(ContextAccessor* accessor_, UI* ui_)
    : QDialog(ui_->GetWindow(mainWindowKey))
    , accessor(accessor_)
    , ui(ui_)
{
#ifdef __DAVAENGINE_WIN32__
    const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Window;
#else
    const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Tool;
#endif
    setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

    VisualScriptEditorDialogDetails::SetStyle();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout* bodyLayout = new QHBoxLayout(this);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    mainLayout->addWidget(CreateToolBar());
    mainLayout->addLayout(bodyLayout);

    QVBoxLayout* flowViewLayout = new QVBoxLayout(this);
    flowViewLayout->setContentsMargins(0, 0, 0, 0);
    flowViewLayout->setSpacing(0);
    bodyLayout->addLayout(flowViewLayout);

    { // add tabbar
        tabBar = new QTabWidget(this);
        tabBar->setTabsClosable(true);
        tabBar->setMovable(false);

        QObject::connect(tabBar, &QTabWidget::currentChanged, this, &VisualScriptEditorDialog::OnTabChanged);
        QObject::connect(tabBar, &QTabWidget::tabCloseRequested, this, &VisualScriptEditorDialog::OnTabCloseRequested);

        flowViewLayout->addWidget(tabBar);
    }

    { // add properties

        updater.reset(new TimerUpdater(1000, TimerUpdater::DisableFastUpdate));
        propertiesView = VisualScriptEditorPropertiesView::CreateProperties(accessor, ui, updater);
        propertiesView->setFixedWidth(300);
        propertiesView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

        bodyLayout->addWidget(propertiesView);
    }

    { // load settings
        VisualScriptEditorDialogSettings* settings = accessor->GetGlobalContext()->GetData<VisualScriptEditorDialogSettings>();
        if (settings->dialogGeometry.isValid())
        {
            setGeometry(settings->dialogGeometry);
            move(settings->dialogGeometry.topLeft());
        }
    }

    NewScript();
}

VisualScriptEditorDialog::~VisualScriptEditorDialog()
{
    while (tabBar->count() > 0)
    {
        tabBar->removeTab(0);
    }

    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    editorData->activeDescriptor = nullptr;
    for (ScriptDescriptor& descriptor : allScripts)
    {
        VisualScriptEditorDialogDetails::DeleteScript(descriptor);
    }
    allScripts.clear();

    DeleteRemovedScripts();

    { // save settings
        VisualScriptEditorDialogSettings* settings = accessor->GetGlobalContext()->GetData<VisualScriptEditorDialogSettings>();
        settings->dialogGeometry = geometry();
    }
}

void VisualScriptEditorDialog::DeleteRemovedScripts()
{
    for (ScriptDescriptor& descriptor : scriptsForDeletion)
    {
        VisualScriptEditorDialogDetails::DeleteScript(descriptor);
    }
    scriptsForDeletion.clear();
}

QToolBar* VisualScriptEditorDialog::CreateToolBar() const
{
    QToolBar* toolbar = new QToolBar();
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    {
        QAction* actionNew = new QAction(QIcon(":/VisualScriptEditor/Icons/new.png"), "New", toolbar);
        QObject::connect(actionNew, &QAction::triggered, this, &VisualScriptEditorDialog::NewScript);

        QAction* actionOpen = new QAction(QIcon(":/VisualScriptEditor/Icons/open.png"), "Open", toolbar);
        QObject::connect(actionOpen, &QAction::triggered, this, &VisualScriptEditorDialog::LoadScript);

        FieldDescriptor scriptFieldDescr;
        scriptFieldDescr.type = ReflectedTypeDB::Get<VisualScriptEditorData>();
        scriptFieldDescr.fieldName = FastName(VisualScriptEditorData::scriptDescriptorFieldName);

        QtAction* actionSave = new QtAction(accessor, QIcon(":/VisualScriptEditor/Icons/save.png"), "Save", toolbar);
        actionSave->SetStateUpdationFunction(QtAction::Enabled, scriptFieldDescr, [](const Any& v) {
            return v.Cast<ScriptDescriptor*>(nullptr) != nullptr;
        });
        QObject::connect(actionSave, &QAction::triggered, this, &VisualScriptEditorDialog::SaveScript);

        FieldDescriptor scriptNodesFieldDescr;
        scriptNodesFieldDescr.type = ReflectedTypeDB::Get<VisualScriptEditorData>();
        scriptNodesFieldDescr.fieldName = FastName(VisualScriptEditorData::scriptNodesFieldName);

        QtAction* actionCompile = new QtAction(accessor, QIcon(":/VisualScriptEditor/Icons/compile.png"), "Compile", toolbar);
        actionCompile->SetStateUpdationFunction(QtAction::Enabled, scriptNodesFieldDescr, [](const Any& v) {
            Vector<VisualScriptNode*> nodes = v.Cast<Vector<VisualScriptNode*>>(Vector<VisualScriptNode*>());
            return nodes.empty() == false;
        });
        QObject::connect(actionCompile, &QAction::triggered, this, &VisualScriptEditorDialog::CompileScript);

        QtAction* actionExecute = new QtAction(accessor, QIcon(":/VisualScriptEditor/Icons/execute.png"), "Execute", toolbar);
        actionExecute->SetStateUpdationFunction(QtAction::Enabled, scriptNodesFieldDescr, [](const Any& v) {
            Vector<VisualScriptNode*> nodes = v.Cast<Vector<VisualScriptNode*>>(Vector<VisualScriptNode*>());
            return nodes.empty() == false;
        });
        QObject::connect(actionExecute, &QAction::triggered, this, &VisualScriptEditorDialog::ExecuteScript);

        QtAction* actionClearSelection = new QtAction(accessor, QIcon(":/VisualScriptEditor/Icons/clear.png"), "Clear Selection", toolbar);
        actionClearSelection->SetStateUpdationFunction(QtAction::Enabled, scriptNodesFieldDescr, [](const Any& v) {
            Vector<VisualScriptNode*> nodes = v.Cast<Vector<VisualScriptNode*>>(Vector<VisualScriptNode*>());
            return nodes.empty() == false;
        });
        //        actionClearSelection->setShortcutContext(Qt::WindowShortcut);
        //        actionClearSelection->setShortcut(Qt::Key_Escape);
        QObject::connect(actionClearSelection, &QAction::triggered, this, &VisualScriptEditorDialog::ClearSelection);

        QtAction* actionDeleteSelection = new QtAction(accessor, QIcon(":/VisualScriptEditor/Icons/delete.png"), "Delete Selection", toolbar);
        actionDeleteSelection->SetStateUpdationFunction(QtAction::Enabled, scriptNodesFieldDescr, [](const Any& v) {
            Vector<VisualScriptNode*> nodes = v.Cast<Vector<VisualScriptNode*>>(Vector<VisualScriptNode*>());
            return nodes.empty() == false;
        });
        actionDeleteSelection->setShortcutContext(Qt::WindowShortcut);
        actionDeleteSelection->setShortcuts(QList<QKeySequence>() << Qt::Key_Delete << Qt::CTRL + Qt::Key_Backspace);
        QObject::connect(actionDeleteSelection, &QAction::triggered, this, &VisualScriptEditorDialog::DeleteSelection);

        toolbar->addAction(actionNew);
        toolbar->addAction(actionOpen);
        toolbar->addAction(actionSave);
        toolbar->addSeparator();
        toolbar->addAction(actionCompile);
        toolbar->addAction(actionExecute);
        toolbar->addSeparator();
        toolbar->addAction(actionClearSelection);
        toolbar->addAction(actionDeleteSelection);
    }

    return toolbar;
}

void VisualScriptEditorDialog::OnTabChanged(int tabIndex)
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    editorData->activeDescriptor = nullptr;

    activeScript = tabIndex;

    if (activeScript >= 0 && activeScript < static_cast<int32>(allScripts.size()))
    {
        editorData->activeDescriptor = &allScripts[activeScript];
    }
}

void VisualScriptEditorDialog::OnTabCloseRequested(int tabIndex)
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    if (tabIndex >= 0 && tabIndex < static_cast<int32>(allScripts.size()))
    {
        if (editorData->activeDescriptor == &allScripts[tabIndex])
        {
            editorData->activeDescriptor = nullptr;
        }

        scriptsForDeletion.push_back(allScripts[tabIndex]);
        delayedDeleteExecutor.DelayedExecute(MakeFunction(this, &VisualScriptEditorDialog::DeleteRemovedScripts));

        auto it = allScripts.begin();
        std::advance(it, tabIndex);
        allScripts.erase(it);
    }
}

void VisualScriptEditorDialog::NewScript()
{
    NewScriptImpl(FilePath());
}

void VisualScriptEditorDialog::LoadScript()
{
    FileDialogParams fParams;
    fParams.filters = QStringLiteral("Visual Script (*.dvs)");
    fParams.title = QStringLiteral("Open Visual Script");

    QString path = ui->GetOpenFileName(mainWindowKey, fParams);
    if (path.isEmpty() == false)
    {
        FilePath scriptPath = path.toStdString();
        NewScriptImpl(scriptPath);
    }
}

void VisualScriptEditorDialog::OpenScriptByPath(const QString& path)
{
    if (path.isEmpty() == false)
    {
        FilePath scriptPath = path.toStdString();
        for (ScriptDescriptor& descriptor : allScripts)
        {
            if (descriptor.scriptPath == scriptPath)
            {
                int newTabIndex = tabBar->indexOf(descriptor.flowView);
                tabBar->setCurrentIndex(newTabIndex);
                return;
            }
        }

        NewScriptImpl(scriptPath);
    }
}

void VisualScriptEditorDialog::NewScriptImpl(const FilePath& scriptPath)
{
    allScripts.emplace_back();

    ScriptDescriptor& descriptor = *allScripts.rbegin();
    descriptor.script = new VisualScript();
    descriptor.reflectionHolder = new VisualScriptEditorReflectionHolder();
    descriptor.isScriptOwner = true;

    descriptor.scriptPath = scriptPath;

    static int32 tabIndex = 1;
    QString tabName = QString("VisualScript %1").arg(tabIndex++);
    if (scriptPath.IsEmpty() == false)
    {
        descriptor.script->Load(scriptPath);
        StoreRecentScriptPathname(scriptPath);
        tabName = QString::fromStdString(scriptPath.GetFilename());
    }

    Vector<Reflection>& reflectedDataModels = descriptor.reflectionHolder->reflectedModels;
    Reflection scriptReflection = Reflection::Create(ReflectedObject(descriptor.script));
    reflectedDataModels.push_back(scriptReflection.GetField(FastName("dataRegistry")));

    std::shared_ptr<VisualScriptRegistryModel> registry = std::make_shared<VisualScriptRegistryModel>(accessor, ui, descriptor.script);
    registry->SetDataContainers(reflectedDataModels);
    registry->BuildStaticModel();

    descriptor.flowScene = new VisualScriptFlowScene(accessor, ui, descriptor.script, registry);
    descriptor.flowView = new VisualScriptFlowView(descriptor.flowScene);

    descriptor.flowScene->LoadRuntime();
    QObject::connect(descriptor.flowScene, &VisualScriptFlowScene::nodeDoubleClicked, this, &VisualScriptEditorDialog::OnNodeDoubleClicked);

    int newTabIndex = tabBar->addTab(descriptor.flowView, tabName);
    tabBar->setCurrentIndex(newTabIndex);
}

void VisualScriptEditorDialog::SaveScript()
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    if (editorData->activeDescriptor->scriptPath.IsEmpty() == true)
    {
        FileDialogParams fParams;
        fParams.filters = QStringLiteral("Visual Script (*.dvs)");
        fParams.title = QStringLiteral("Save Visual Script");

        QString path = ui->GetSaveFileName(mainWindowKey, fParams);
        if (path.isEmpty() == true)
        {
            return;
        }

        editorData->activeDescriptor->scriptPath = path.toStdString();
        tabBar->setTabText(tabBar->currentIndex(), QString::fromStdString(editorData->activeDescriptor->scriptPath.GetFilename()));
    }

    editorData->activeDescriptor->flowScene->SaveRuntime();
    editorData->activeDescriptor->script->Save(editorData->activeDescriptor->scriptPath);

    NotificationParams notifParams;
    notifParams.title = "Information";
    notifParams.message.message = editorData->activeDescriptor->scriptPath.GetStringValue() + " saved";
    ui->ShowNotification(mainWindowKey, notifParams);
}

void VisualScriptEditorDialog::CompileScript()
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    editorData->activeDescriptor->script->Compile();
}

void VisualScriptEditorDialog::ExecuteScript()
{
    try
    {
        CompileScript();

        VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();

        std::vector<QtNodes::Node*> nodes = editorData->activeDescriptor->flowScene->selectedNodes();
        for (QtNodes::Node* n : nodes)
        {
            VisualScriptNodeModel* dataModel = static_cast<VisualScriptNodeModel*>(n->nodeDataModel());
            VisualScriptEventNode* eventNode = dynamic_cast<VisualScriptEventNode*>(dataModel->GetScriptNode());
            if (eventNode != nullptr)
            {
                editorData->activeDescriptor->script->Execute(eventNode->GetEventName(), Reflection());
            }
            else
            {
                DAVA_THROW(Exception, "Can execute only event nodes");
            }
        }
    }
    catch (Exception& exception)
    {
        Logger::Error(exception.what());
    }
}

void VisualScriptEditorDialog::StoreRecentScriptPathname(const FilePath& scriptPath)
{
    VisualScriptEditorDialogSettings* settings = accessor->GetGlobalContext()->GetData<VisualScriptEditorDialogSettings>();
    DVASSERT(settings != nullptr);

    String scriptPathStr = scriptPath.GetStringValue();
    auto it = std::find(settings->recentScripts.begin(), settings->recentScripts.end(), scriptPathStr);
    if (it == settings->recentScripts.end())
    {
        settings->recentScripts.insert(settings->recentScripts.begin(), scriptPathStr);
        if (settings->recentScripts.size() > 10)
        {
            settings->recentScripts.pop_back();
        }
    }
}

void VisualScriptEditorDialog::OnNodeDoubleClicked(QtNodes::Node& n)
{
    VisualScriptNodeModel* dataModel = static_cast<VisualScriptNodeModel*>(n.nodeDataModel());
    VisualScriptAnotherScriptNode* scriptNode = dynamic_cast<VisualScriptAnotherScriptNode*>(dataModel->GetScriptNode());
    if (scriptNode != nullptr)
    {
        const FilePath& scriptPath = scriptNode->GetScriptFilepath();

        int32 count = static_cast<int32>(allScripts.size());
        for (int32 tabIndex = 0; tabIndex < count; ++tabIndex)
        {
            if (allScripts[tabIndex].scriptPath == scriptPath)
            {
                tabBar->setCurrentIndex(tabIndex);
                return;
            }
        }

        NewScriptImpl(scriptPath);
    }
}

void VisualScriptEditorDialog::ClearSelection()
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    DVASSERT(editorData->activeDescriptor != nullptr);

    editorData->activeDescriptor->flowView->ClearSelectedNodes();
}

void VisualScriptEditorDialog::DeleteSelection()
{
    VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
    DVASSERT(editorData->activeDescriptor != nullptr);

    editorData->activeDescriptor->flowView->deleteSelectedNodes();
}

} // DAVA
