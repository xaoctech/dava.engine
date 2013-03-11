#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"
#include "Entity/Component.h"
#include "Main/mainwindow.h"

#include "DockProperties/PropertyEditor.h"
#include "QtPropertyEditor/QtPropertyItem.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataIntrospection.h"
#include "QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"

#include "PropertyEditorStateHelper.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */)
	: QtPropertyEditor(parent)
	, hideReadOnly(false)
	, curNode(NULL)
	, treeStateHelper(this, this->curModel)
{
	// global scene manager signals
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneActivated(SceneData *)), this, SLOT(sceneActivated(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneChanged(SceneData *)), this, SLOT(sceneChanged(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneReleased(SceneData *)), this, SLOT(sceneReleased(SceneData *)));
	QObject::connect(SceneDataManager::Instance(), SIGNAL(SceneNodeSelected(SceneData *, DAVA::SceneNode *)), this, SLOT(sceneNodeSelected(SceneData *, DAVA::SceneNode *)));

	// MainWindow actions
	QObject::connect(QtMainWindow::Instance()->GetUI()->actionPropHideReadonly, SIGNAL(triggered()), this, SLOT(actionHideReadOnly()));

	hideReadOnly = QtMainWindow::Instance()->GetUI()->actionPropHideReadonly->isChecked();

	posSaver.Attach(this, "DocPropetyEditor");
	posSaver.LoadState(this);
}

PropertyEditor::~PropertyEditor()
{
	posSaver.SaveState(this);
	SafeRelease(curNode);
}

void PropertyEditor::SetNode(DAVA::SceneNode *node)
{
	// Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);

	SafeRelease(curNode);
	curNode = SafeRetain(node);

	RemovePropertyAll();
	if(NULL != curNode)
	{
        AppendIntrospectionInfo(curNode, curNode->GetTypeInfo());

		for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
        {
            Component *component = curNode->GetComponent(i);
            if(component)
            {
                AppendIntrospectionInfo(component, component->GetTypeInfo());
            }
        }
	}

	// Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

void PropertyEditor::AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo *info)
{
	if(NULL != info)
	{
		bool hasMembers = false;
		const IntrospectionInfo *currentInfo = info;

		// check if there are any memebers
		while (NULL != currentInfo)
		{
			if(currentInfo->MembersCount() > 0)
			{
				hasMembers = true;
				break;
			}
			currentInfo = currentInfo->BaseInfo();
		}

        if(hasMembers)
        {
			int hasFlags = DAVA::INTROSPECTION_EDITOR;
			int hasNotFlags = 0;

			if(hideReadOnly) hasNotFlags |= DAVA::INTROSPECTION_EDITOR_READONLY;

			QtPropertyData* propData = new QtPropertyDataIntrospection(object, currentInfo, hasFlags, hasNotFlags);

			if(propData->ChildCount() > 0)
			{
				QPair<QtPropertyItem*, QtPropertyItem*> prop = AppendProperty(currentInfo->Name(), propData);
            
	            prop.first->setBackground(QBrush(QColor(Qt::lightGray)));
		        prop.second->setBackground(QBrush(QColor(Qt::lightGray)));
			}
			else
			{
				delete propData;
			}
        }
    }
}

void PropertyEditor::sceneChanged(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneActivated(SceneData *sceneData)
{
	if(NULL != sceneData)
	{
		SetNode(sceneData->GetSelectedNode());
	}
}

void PropertyEditor::sceneReleased(SceneData *sceneData)
{ }

void PropertyEditor::sceneNodeSelected(SceneData *sceneData, DAVA::SceneNode *node)
{
	SetNode(node);
}

void PropertyEditor::actionHideReadOnly()
{
	QAction *hideAction = dynamic_cast<QAction *>(QObject::sender());
	if(NULL != hideAction)
	{
		hideReadOnly = hideAction->isChecked();
		SetNode(curNode);
	}
}