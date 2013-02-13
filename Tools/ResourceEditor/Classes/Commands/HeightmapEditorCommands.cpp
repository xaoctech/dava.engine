#include "HeightmapEditorCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/GUIState.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../SceneEditor/EditorBodyControl.h"

#include "Utils/Random.h"

//Show/Hide Heightmap Editor
CommandHeightmapEditor::CommandHeightmapEditor()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandHeightmapEditor::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->HeightmapTriggered();
        GUIState::Instance()->SetNeedUpdatedToolsMenu(true);
        GUIState::Instance()->SetNeedUpdatedToolbar(true);
    }

    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RebuildSceneGraph();
}


CommandDrawHeightmap::CommandDrawHeightmap()
:	Command(COMMAND_UNDO_REDO)
{
	commandName = "Heightmap Change";
	redoFilename = "";

	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor)
	{
		Heightmap* heightmap = editor->GetHeightmap();
		undoFilename = SaveHeightmap(heightmap);
	}
}

CommandDrawHeightmap::~CommandDrawHeightmap()
{
	FileSystem::Instance()->DeleteFile(undoFilename);
	FileSystem::Instance()->DeleteFile(redoFilename);
}

void CommandDrawHeightmap::Execute()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	Heightmap* heightmap = editor->GetHeightmap();

	if (redoFilename == "")
	{
		redoFilename = SaveHeightmap(heightmap);
	}
	else
	{
		
		heightmap->Load(redoFilename);
		editor->UpdateHeightmap(heightmap);
	}
}

void CommandDrawHeightmap::Cancel()
{
	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor)
	{
		Heightmap* heightmap = editor->GetHeightmap();

		heightmap->Load(undoFilename);
		editor->UpdateHeightmap(heightmap);
	}
}

LandscapeEditorHeightmap* CommandDrawHeightmap::GetEditor()
{
	LandscapeEditorHeightmap* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorHeightmap*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_HEIGHTMAP));
	}

	return editor;
}

String CommandDrawHeightmap::TimeString()
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);

    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

    return timeString;
}

String CommandDrawHeightmap::SaveHeightmap(Heightmap* heightmap)
{
	String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
	String folderPathname = documentsPath + "History";
	FileSystem::Instance()->CreateDirectory(folderPathname);
	folderPathname = folderPathname + "/Heightmap";
	FileSystem::Instance()->CreateDirectory(folderPathname);

	FileList* fileList = new FileList(folderPathname);

	bool validFileName = false;
	String filename;
	String time = TimeString();
	uint32 num = 0;
	do
	{
		filename = time;
		if (num)
		{
			filename += Format(" (%d)", num);
		}
		filename += Heightmap::FileExtension();

		uint32 i = 0;
		for (; i < fileList->GetFileCount(); ++i)
		{
			if (fileList->GetFilename(i) == filename)
			{
				++num;
				break;
			}
		}
		if (i >= fileList->GetFileCount())
			validFileName = true;
	} while (!validFileName);

	filename = folderPathname + "/" + filename;
	heightmap->Save(filename);

	SafeRelease(fileList);

	return filename;
}
