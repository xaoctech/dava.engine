#include "Classes/Qt/Tools/AddSwitchEntityDialog/AddSwitchEntityDialog.h"
#include "Classes/Qt/Tools/AddSwitchEntityDialog/SwitchEntityCreator.h"
#include "Classes/Qt/Tools/MimeDataHelper/MimeDataHelper.h"
#include "Classes/Qt/Tools/SelectPathWidget/SelectEntityPathWidget.h"

#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Commands/EntityAddCommand.h>
#include <REPlatform/Commands/EntityRemoveCommand.h>

#include "ui_BaseAddEntityDialog.h"

#include <QtTools/ConsoleWidget/PointerSerializer.h>

#include <TArc/Core/Deprecated.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QLabel>
#include "REPlatform/Global/StringConstants.h"

namespace AddSwitchEntityDialogDetails
{
DAVA::SceneEditor2* GetActiveScene()
{
    DAVA::SceneData* data = DAVA::Deprecated::GetActiveDataNode<DAVA::SceneData>();
    if (data != nullptr)
    {
        return data->GetScene().Get();
    }
    return nullptr;
}
}

AddSwitchEntityDialog::AddSwitchEntityDialog(QWidget* parent)
    : BaseAddEntityDialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    DAVA::ProjectManagerData* data = DAVA::Deprecated::GetDataNode<DAVA::ProjectManagerData>();
    DVASSERT(data != nullptr);
    DAVA::FilePath defaultPath(data->GetDataSource3DPath());

    DAVA::SceneEditor2* scene = AddSwitchEntityDialogDetails::GetActiveScene();
    if (scene != nullptr)
    {
        DAVA::FilePath scenePath = scene->GetScenePath();
        if (scenePath.Exists())
        {
            defaultPath = scenePath.GetDirectory();
        }
    }

    SelectEntityPathWidget* firstWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(), "");
    SelectEntityPathWidget* secondWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(), "");
    SelectEntityPathWidget* thirdWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(), "");

    AddControlToUserContainer(firstWidget, "First Entity:");
    AddControlToUserContainer(secondWidget, "Second Entity:");
    AddControlToUserContainer(thirdWidget, "Third Entity:");

    pathWidgets.push_back(firstWidget);
    pathWidgets.push_back(secondWidget);
    pathWidgets.push_back(thirdWidget);

    propEditor->setVisible(false);
    propEditor->setMinimumHeight(0);
    propEditor->setMaximumSize(propEditor->maximumWidth(), 0);
}

AddSwitchEntityDialog::~AddSwitchEntityDialog()
{
    RemoveAllControlsFromUserContainer();
    Q_FOREACH (SelectEntityPathWidget* widget, pathWidgets)
    {
        delete widget;
    }
}

void AddSwitchEntityDialog::CleanupPathWidgets()
{
    Q_FOREACH (SelectEntityPathWidget* widget, pathWidgets)
    {
        widget->EraseWidget();
    }
}

void AddSwitchEntityDialog::GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, DAVA::SceneEditor2* editor)
{
    Q_FOREACH (SelectEntityPathWidget* widget, pathWidgets)
    {
        DAVA::Entity* entity = widget->GetOutputEntity(editor);
        if (entity)
        {
            entities.push_back(entity);
        }
    }
}

void AddSwitchEntityDialog::accept()
{
    DAVA::SceneEditor2* scene = AddSwitchEntityDialogDetails::GetActiveScene();
    if (scene == nullptr)
    {
        CleanupPathWidgets();
        return;
    }

    DAVA::Vector<DAVA::Entity*> vector;
    GetPathEntities(vector, scene);

    if (vector.empty())
    {
        DAVA::Logger::Error(DAVA::ResourceEditor::ADD_SWITCH_NODE_DIALOG_NO_CHILDREN.c_str());
        return;
    }

    CleanupPathWidgets();

    SwitchEntityCreator creator;

    bool canCreateSwitch = true;

    DAVA::uint32 switchCount = (DAVA::uint32)vector.size();
    for (DAVA::uint32 i = 0; i < switchCount; ++i)
    {
        if (creator.HasSwitchComponentsRecursive(vector[i]))
        {
            canCreateSwitch = false;
            DAVA::Logger::Error("Can't create switch in switch: %s%s", vector[i]->GetName().c_str(),
                                PointerSerializer::FromPointer(vector[i]).c_str());
            DAVA::Logger::Error(DAVA::ResourceEditor::ADD_SWITCH_NODE_DIALOG_DENY_SRC_SWITCH.c_str());
            return;
        }
        if (!creator.HasRenderObjectsRecursive(vector[i]))
        {
            canCreateSwitch = false;
            DAVA::Logger::Error("Entity '%s' hasn't mesh render objects%s", vector[i]->GetName().c_str(),
                                PointerSerializer::FromPointer(vector[i]).c_str());
            DAVA::Logger::Error(DAVA::ResourceEditor::ADD_SWITCH_NODE_DIALOG_NO_RENDER_OBJECTS.c_str());
            return;
        }
    }

    if (canCreateSwitch)
    {
        scene->BeginBatch("Unite entities into switch entity.", switchCount + 1);
        for (DAVA::uint32 i = 0; i < switchCount; ++i)
        {
            vector[i]->Retain();
            scene->Exec(std::unique_ptr<DAVA::Command>(new DAVA::EntityRemoveCommand(vector[i])));
        }

        DAVA::Entity* switchEntity = creator.CreateSwitchEntity(vector);
        scene->Exec(std::unique_ptr<DAVA::Command>(new DAVA::EntityAddCommand(switchEntity, scene)));

        for (DAVA::uint32 i = 0; i < switchCount; ++i)
        {
            vector[i]->Release();
        }

        scene->EndBatch();
        DAVA::SafeRelease(switchEntity);
    }

    BaseAddEntityDialog::accept();
}

void AddSwitchEntityDialog::reject()
{
    CleanupPathWidgets();
    BaseAddEntityDialog::reject();
}

void AddSwitchEntityDialog::FillPropertyEditorWithContent()
{
}
