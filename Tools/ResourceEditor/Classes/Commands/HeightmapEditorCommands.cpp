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
	redoFilename = "";

	LandscapeEditorHeightmap* editor = GetEditor();
	if (editor)
	{
		Heightmap* heightmap;
		editor->GetHeightmap(&heightmap);
		undoFilename = SaveHeightmap(heightmap, "_" + GetRandomString(10));
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

	Heightmap* heightmap;
	editor->GetHeightmap(&heightmap);

	if (redoFilename == "")
	{
		redoFilename = SaveHeightmap(heightmap, "_" + GetRandomString(10));
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
		Heightmap* heightmap;
		editor->GetHeightmap(&heightmap);

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

String CommandDrawHeightmap::SaveHeightmap(Heightmap* heightmap, String suffix)
{
	String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
	String folderPathname = documentsPath + "History";
	FileSystem::Instance()->CreateDirectory(folderPathname);
	folderPathname = folderPathname + "/Heightmap";
	FileSystem::Instance()->CreateDirectory(folderPathname);

	String filename = folderPathname + "/" + TimeString() + suffix + Heightmap::FileExtension();
	heightmap->Save(filename);

	return filename;
}

String CommandDrawHeightmap::GetRandomString(uint32 len)
{
	String res = "";
	while (len--)
	{
		uint32 n = Random::Instance()->Rand(32);
		if (n >= 10)
			res += 'a' + n - 10;
		else
			res += '0' + n;
	}

	return res;
}
