#include "RunActionEventWidget.h"

#include <QStringList>
#include <QSet>
#include <QCompleter>

#include "ui_RunActionEventWidget.h"

#include "DAVAEngine.h"
#include "Scene3D/Components/ActionComponent.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Scene/SceneEditor2.h"


RunActionEventWidget::RunActionEventWidget(QWidget *parent)
    : QWidget(parent)
    , ui( new Ui::RunActionEventWidget() )
    , scene(NULL)
{
    ui->setupUi(this);
    setObjectName("RunActionEventWidget");

    connect( ui->eventType, SIGNAL( currentIndexChanged( int ) ), SLOT( OnTypeChanged() ) );
    connect( ui->run, SIGNAL( clicked() ), SLOT( OnInvoke() ) );

    ui->eventType->addItem( "Switch", ActionComponent::Action::EVENT_SWITCH_CHANGED );
    ui->eventType->addItem( "Added", ActionComponent::Action::EVENT_ADDED_TO_SCENE );
    ui->eventType->addItem( "User", ActionComponent::Action::EVENT_CUSTOM );

    editorIdMap[ActionComponent::Action::EVENT_SWITCH_CHANGED] = 0;
    editorIdMap[ActionComponent::Action::EVENT_ADDED_TO_SCENE] = 1;
    editorIdMap[ActionComponent::Action::EVENT_CUSTOM] = 2;

    autocompleteModel = new QStringListModel(this);
    QCompleter *completer = new QCompleter(this);
    completer->setModel(autocompleteModel);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    //completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui->name->setCompleter(completer);
    ui->name->installEventFilter(this); // For auto-popup on focus

    connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
}

RunActionEventWidget::~RunActionEventWidget()
{
}

bool RunActionEventWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->name)
    {
        switch(event->type())
        {
        case QEvent::FocusIn:
            {
                QAbstractItemView *popup = ui->name->completer()->popup();
                if (!popup->isVisible())
                {
                    ui->name->completer()->complete();
                    popup->show();
                }
            }
            break;
        default:
            break;
        }
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void RunActionEventWidget::OnTypeChanged()
{
    const int eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toInt();
    const int editorindex = editorIdMap[eventTypeId];
    DVASSERT(editorindex < ui->stackedWidget->count());

    ui->stackedWidget->setCurrentIndex(editorindex);
}

void RunActionEventWidget::OnInvoke()
{
    const uint eventTypeId = ui->eventType->itemData(ui->eventType->currentIndex()).toUInt();
    SceneEditor2 *editor = QtMainWindow::Instance()->GetCurrentScene();
    if (editor == NULL)
        return ;

    const uint32 switchIndex = ui->switchIndex->value();
    const FastName name(ui->name->text().toStdString().c_str());

    const EntityGroup& selection = editor->selectionSystem->GetSelection();
    for (size_t i = 0; i < selection.Size(); i++)
    {
        Entity* entity = selection.GetEntity(i);
        ActionComponent *component = static_cast<ActionComponent *>(entity->GetComponent(Component::ACTION_COMPONENT));
        if (!component)
            continue;
        const uint32 nEvents = component->GetCount();
        for (uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == eventTypeId)
            {
                switch ( eventTypeId )
                {
                case ActionComponent::Action::EVENT_SWITCH_CHANGED:
                    if (act.switchIndex == switchIndex)
                    {
                        component->StartSwitch(switchIndex);
                    }
                    break;
                case ActionComponent::Action::EVENT_ADDED_TO_SCENE:
                    component->StartAdd();
                    break;
                case ActionComponent::Action::EVENT_CUSTOM:
                    if (act.userEventId == name)
                    {
                        component->StartUser(name);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

}

void RunActionEventWidget::sceneActivated(SceneEditor2 *_scene)
{
    scene = _scene;
    sceneSelectionChanged(scene, NULL, NULL);
}

void RunActionEventWidget::sceneSelectionChanged(SceneEditor2 *_scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (scene == NULL)
    {
        autocompleteModel->setStringList(QStringList());
        return ;
    }

    if (scene != _scene)
    {
        return ;
    }

    QSet< QString > nameSet;

    const EntityGroup& selection = scene->selectionSystem->GetSelection();
    for (size_t i = 0; i < selection.Size(); i++)
    {
        Entity* entity = selection.GetEntity(i);
        ActionComponent *component = static_cast<ActionComponent *>(entity->GetComponent(Component::ACTION_COMPONENT));
        if (!component)
            continue;
        const uint32 nEvents = component->GetCount();
        for (uint32 componentIdx = 0; componentIdx < nEvents; componentIdx++)
        {
            ActionComponent::Action& act = component->Get(componentIdx);
            if (act.eventType == ActionComponent::Action::EVENT_CUSTOM)
            {
                nameSet.insert(QString(act.userEventId.c_str()));
            }
        }
    }

    const QStringList& nameList = nameSet.toList();
    autocompleteModel->setStringList(nameList);
}
