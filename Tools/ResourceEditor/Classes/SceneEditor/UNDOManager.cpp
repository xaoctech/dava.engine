#include "UNDOManager.h"
#include "Scene3D/Heightmap.h"


#pragma mark  --UNDOAction
UNDOAction::UNDOAction()
{
    type = ACTION_NONE;
    ID = -1;
    filePathname = "";
}

#pragma mark  --UNDOManager
UNDOManager::UNDOManager()
    :       actionCounter(0)
{
}

UNDOManager::~UNDOManager()
{
    List<UNDOAction *>::iterator it = actionsHistory.begin();
    List<UNDOAction *>::const_iterator endIt = actionsHistory.end();

    for(; it != endIt; ++it)
    {
        SafeRelease(*it);
    }
    actionsHistory.clear();
    
    String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
    String historyPath = documentsPath + "History";
    FileSystem::Instance()->DeleteDirectory(historyPath, true);
}

void UNDOManager::CheckHistoryLength()
{
    if(UNDO_HISTORY_SIZE == actionsHistory.size())
    {
        List<UNDOAction *>::iterator it = actionsHistory.begin();
        SafeRelease(*it);
        actionsHistory.erase(it);
    }
}

void UNDOManager::SaveHightmap(Heightmap *heightmap)
{
    DVASSERT(heightmap);
    
    CheckHistoryLength();

    String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
    String folderPathname = documentsPath + "History";
    FileSystem::Instance()->CreateDirectory(folderPathname);
    folderPathname = folderPathname + "/Heightmap";
    FileSystem::Instance()->CreateDirectory(folderPathname);
    
    UNDOAction *action = new UNDOAction();
    action->type = UNDOAction::ACTION_HEIGHTMAP;
    action->ID = actionCounter++;
    action->filePathname = folderPathname + "/" + TimeString() + Heightmap::FileExtension();
 
    heightmap->Save(action->filePathname);

    actionsHistory.push_back(action);
}

void UNDOManager::UndoHeightmap(Heightmap *heightmap)
{
    if(actionsHistory.size())
    {
        List<UNDOAction *>::iterator it = actionsHistory.end();
        --it;
        
        heightmap->Load((*it)->filePathname);
        
        FileSystem::Instance()->DeleteFile((*it)->filePathname);
        
        SafeRelease(*it);
        actionsHistory.erase(it);
    }
}

UNDOAction::eActionType UNDOManager::GetLastUNDOAction()
{
    UNDOAction::eActionType retAction = UNDOAction::ACTION_NONE;
    
    if(actionsHistory.size())
    {
        List<UNDOAction *>::iterator it = actionsHistory.end();
        --it;
        retAction = (*it)->type;
    }
    
    return retAction;
}


String UNDOManager::TimeString()
{
    time_t now = time(0);
    tm* utcTime = localtime(&now);
    
    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",   
                                            utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday, 
                                            utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

    return timeString;
}
