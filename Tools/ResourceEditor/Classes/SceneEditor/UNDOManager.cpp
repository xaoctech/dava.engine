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
    :   actionCounter(0)
    ,   actionDirection(DIRECTION_NONE)
{
    String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
    String historyPath = documentsPath + "History";
    FileSystem::Instance()->DeleteDirectory(historyPath, true);
}

UNDOManager::~UNDOManager()
{
    ReleaseHistory(actionsHistoryUNDO);
    ReleaseHistory(actionsHistoryREDO);
    
    String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
    String historyPath = documentsPath + "History";
    FileSystem::Instance()->DeleteDirectory(historyPath, true);
}

void UNDOManager::CheckHistoryLength()
{
    if(UNDO_HISTORY_SIZE == actionsHistoryUNDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryUNDO.begin();
        SafeRelease(*it);
        actionsHistoryUNDO.erase(it);
    }
}

UNDOAction * UNDOManager::CreateHeightmapAction(Heightmap *heightmap)
{
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

    return action;
}



void UNDOManager::SaveHightmap(Heightmap *heightmap)
{
    DVASSERT(heightmap);
    
    CheckHistoryLength();

    UNDOAction *action = CreateHeightmapAction(heightmap);
    actionsHistoryUNDO.push_back(action);
    
    ReleaseHistory(actionsHistoryREDO);
}

void UNDOManager::UndoHeightmap(Heightmap *heightmap)
{
    int32 szu = actionsHistoryUNDO.size();
    int32 szr = actionsHistoryREDO.size();
    
    if(actionsHistoryUNDO.size())
    {
        if(0 == actionsHistoryREDO.size())
        {
            UNDOAction *action = CreateHeightmapAction(heightmap);
            actionsHistoryREDO.push_front(action);
        }
        
        List<UNDOAction *>::iterator it = actionsHistoryUNDO.end();
        --it;
        
        heightmap->Load((*it)->filePathname);
        
        if(1 < actionsHistoryUNDO.size())
        {
            actionsHistoryREDO.push_front(*it);
        }
        else 
        {
            FileSystem::Instance()->DeleteFile((*it)->filePathname);
            SafeRelease(*it);
        }
        actionsHistoryUNDO.erase(it);
        
        actionDirection = DIRECTION_UNDO;
    }
}

void UNDOManager::RedoHeightmap(Heightmap *heightmap)
{
    int32 szu = actionsHistoryUNDO.size();
    int32 szr = actionsHistoryREDO.size();

    if(actionsHistoryREDO.size())
    {
        if(0 == actionsHistoryUNDO.size())
        {
            UNDOAction *action = CreateHeightmapAction(heightmap);
            actionsHistoryUNDO.push_back(action);
        }
        
        List<UNDOAction *>::iterator it = actionsHistoryREDO.begin();
        heightmap->Load((*it)->filePathname);
        
        if(1 < actionsHistoryREDO.size())
        {
            actionsHistoryUNDO.push_back(*it);
        }
        else 
        {
            FileSystem::Instance()->DeleteFile((*it)->filePathname);
            SafeRelease(*it);
        }
        actionsHistoryREDO.erase(it);
        
        actionDirection = DIRECTION_REDO;
    }
}


UNDOAction::eActionType UNDOManager::GetLastUNDOAction()
{
    UNDOAction::eActionType retAction = UNDOAction::ACTION_NONE;
    
    if(actionsHistoryUNDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryUNDO.end();
        --it;
        retAction = (*it)->type;
    }
    
    return retAction;
}


UNDOAction::eActionType UNDOManager::GetFirstREDOAction()
{
    UNDOAction::eActionType retAction = UNDOAction::ACTION_NONE;
    
    if(actionsHistoryREDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryREDO.begin();
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

void UNDOManager::ClearHistory(UNDOAction::eActionType forAction)
{
    ClearHistory(actionsHistoryUNDO, forAction);
    ClearHistory(actionsHistoryREDO, forAction);
}

void UNDOManager::ClearHistory(List<UNDOAction *> &actionsHistory, UNDOAction::eActionType forAction)
{
    List<UNDOAction *>::iterator it = actionsHistory.begin();
    List<UNDOAction *>::const_iterator endIt = actionsHistory.end();
    
    for(; it != endIt; )
    {
        if(forAction == (*it)->type)
        {
            FileSystem::Instance()->DeleteFile((*it)->filePathname);
            SafeRelease(*it);
            
            it = actionsHistory.erase(it);
        }
        else 
        {
            ++it;
        }
    }
}


void UNDOManager::ReleaseHistory(List<UNDOAction *> &actionsHistory)
{
    List<UNDOAction *>::iterator it = actionsHistory.begin();
    List<UNDOAction *>::const_iterator endIt = actionsHistory.end();
    
    for(; it != endIt; ++it)
    {
        SafeRelease(*it);
    }
    actionsHistory.clear();
}


