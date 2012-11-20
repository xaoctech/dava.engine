#include "UNDOManager.h"
#include "Scene3D/Heightmap.h"

struct VisibilityActionDataStruct
{
	Image* image;
	bool isVisibilityPointSet;
	Vector2 visibilityPointPos;
};

UNDOAction::UNDOAction()
{
    type = ACTION_NONE;
    ID = -1;
    filePathname = "";
    
    actionData = NULL;
}

UNDOAction::~UNDOAction()
{
	SafeRelease(actionData);
}


UNDOManager::UNDOManager()
    :   actionCounter(0)
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
    if(1 < actionsHistoryUNDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryUNDO.end();
        --it;
        
        actionsHistoryREDO.push_front(*it);
        actionsHistoryUNDO.erase(it);
        
        it = actionsHistoryUNDO.end();
        --it;
        heightmap->Load((*it)->filePathname);
    }
}

void UNDOManager::RedoHeightmap(Heightmap *heightmap)
{
    if(actionsHistoryREDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryREDO.begin();
        heightmap->Load((*it)->filePathname);
        
        actionsHistoryUNDO.push_back(*it);
        actionsHistoryREDO.erase(it);
    }
}


UNDOAction * UNDOManager::CreateTilemaskAction(Texture *tilemask)
{
    return CreateTextureAction(tilemask, UNDOAction::ACTION_TILEMASK);
}


void UNDOManager::SaveTilemask(Texture *tilemask)
{
    SaveTexture(tilemask, UNDOAction::ACTION_TILEMASK);
}

Texture * UNDOManager::UndoTilemask()
{
    return UNDOManager::UndoTexture();
}

Texture * UNDOManager::RedoTilemask()
{
    return  UNDOManager::RedoTexture();
}

UNDOAction * UNDOManager::CreateColorizeAction(Texture *colTex)
{
	return CreateTextureAction(colTex, UNDOAction::ACTION_COLORIZE);
}

void UNDOManager::SaveColorize(Texture *colTex)
{
    SaveTexture(colTex, UNDOAction::ACTION_COLORIZE);
}

Texture * UNDOManager::UndoColorize()
{
    return  UNDOManager::UndoTexture();
}

Texture * UNDOManager::RedoColorize()
{
    return  UNDOManager::RedoTexture();
}

UNDOAction* UNDOManager::CreateVisibilityAction(Texture *visibilityAreaTexture, bool visibilityPointSet, const Vector2 &visibilityPoint)
{
	UNDOAction* action = new UNDOAction();
	action->type = UNDOAction::ACTION_VISIBILITY;
	action->ID = actionCounter++;
	action->filePathname = "";

	Image* image = visibilityAreaTexture->CreateImageFromMemory();

	VisibilityActionDataStruct* data = new VisibilityActionDataStruct;
	data->image = image;
	data->isVisibilityPointSet = visibilityPointSet;
	data->visibilityPointPos = visibilityPoint;
	action->actionData = data;

	return action;
}

void UNDOManager::SaveVisibility(Texture *visibilityAreaTexture, bool visibilityPointSet, const Vector2 &visibilityPoint)
{
	DVASSERT(visibilityAreaTexture);

	CheckHistoryLength();

	UNDOAction *action = CreateVisibilityAction(visibilityAreaTexture, visibilityPointSet, visibilityPoint);
	actionsHistoryUNDO.push_back(action);

	ReleaseHistory(actionsHistoryREDO);
}

void UNDOManager::UndoVisibility(Texture** visibilityAreaTexture, bool* visibilityPointSet, Vector2* visibilityPoint)
{
	if(1 < actionsHistoryUNDO.size())
	{
		List<UNDOAction*>::iterator it = actionsHistoryUNDO.end();
		--it;

		actionsHistoryREDO.push_front(*it);
		actionsHistoryUNDO.erase(it);

		it = actionsHistoryUNDO.end();
		--it;

		RenderManager::Instance()->LockNonMain();
		VisibilityActionDataStruct* data = (VisibilityActionDataStruct*)((*it)->actionData);
		Image* image = (Image*)(data->image);
		*visibilityAreaTexture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
		RenderManager::Instance()->UnlockNonMain();
		(*visibilityAreaTexture)->isAlphaPremultiplied = image->isAlphaPremultiplied;

		*visibilityPointSet = data->isVisibilityPointSet;
		*visibilityPoint = data->visibilityPointPos;
	}
}

void UNDOManager::RedoVisibility(Texture** visibilityAreaTexture, bool* visibilityPointSet, Vector2* visibilityPoint)
{
	if(actionsHistoryREDO.size() != 0)
	{
		List<UNDOAction*>::iterator it = actionsHistoryREDO.begin();

		RenderManager::Instance()->LockNonMain();
		VisibilityActionDataStruct* data = (VisibilityActionDataStruct*)((*it)->actionData);
		Image* image = (Image*)(data->image);
		*visibilityAreaTexture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
		RenderManager::Instance()->UnlockNonMain();
		(*visibilityAreaTexture)->isAlphaPremultiplied = image->isAlphaPremultiplied;

		*visibilityPointSet = data->isVisibilityPointSet;
		*visibilityPoint = data->visibilityPointPos;

		actionsHistoryUNDO.push_back(*it);
		actionsHistoryREDO.erase(it);
	}
}


Texture * UNDOManager::UndoTexture()
{
    Texture *tex = NULL;
    
    if(1 < actionsHistoryUNDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryUNDO.end();
        --it;
        
        actionsHistoryREDO.push_front(*it);
        actionsHistoryUNDO.erase(it);
        
        it = actionsHistoryUNDO.end();
        --it;

        RenderManager::Instance()->LockNonMain();
        Image *image = (Image *)((*it)->actionData);
        tex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
        RenderManager::Instance()->UnlockNonMain();
        tex->isAlphaPremultiplied = image->isAlphaPremultiplied;
    }
    return tex;
}

Texture * UNDOManager::RedoTexture()
{
    Texture *tex = NULL;
    if(actionsHistoryREDO.size())
    {
        List<UNDOAction *>::iterator it = actionsHistoryREDO.begin();

        RenderManager::Instance()->LockNonMain();
        Image *image = (Image *)((*it)->actionData);
        tex = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight());
        RenderManager::Instance()->UnlockNonMain();
        tex->isAlphaPremultiplied = image->isAlphaPremultiplied;
        
        actionsHistoryUNDO.push_back(*it);
        actionsHistoryREDO.erase(it);
    }

    return tex;
}

UNDOAction * UNDOManager::CreateTextureAction(Texture *tex, UNDOAction::eActionType type)
{
	String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
    String folderPathname = documentsPath + "History";
    FileSystem::Instance()->CreateDirectory(folderPathname);
    folderPathname = folderPathname ;
	switch (type)
	{
		case UNDOAction::ACTION_COLORIZE:
			folderPathname += "/Colorize";
			break;
		case UNDOAction::ACTION_TILEMASK:
			folderPathname += "/Tilemask";
			break;
        default:
            break;
	}
    FileSystem::Instance()->CreateDirectory(folderPathname);
    
    UNDOAction *action = new UNDOAction();
    action->type = type;
    action->ID = actionCounter++;
    action->filePathname = "";
    action->actionData = tex->CreateImageFromMemory();
    
    return action;
}

void UNDOManager::SaveTexture(Texture *tex, UNDOAction::eActionType type)
{
	DVASSERT(tex);
    
    CheckHistoryLength();
    
    UNDOAction *action = NULL;
	switch (type)
	{
		case UNDOAction::ACTION_COLORIZE:
			action = CreateColorizeAction(tex);
			break;
		case UNDOAction::ACTION_TILEMASK:
			action = CreateTilemaskAction(tex);
			break;
        default:
            break;
	}
	
	if(!action)
	{
		return;
	}
    actionsHistoryUNDO.push_back(action);
    
    ReleaseHistory(actionsHistoryREDO);
}

UNDOAction::eActionType UNDOManager::GetLastUNDOAction()
{
    UNDOAction::eActionType retAction = UNDOAction::ACTION_NONE;
    
    if(1 < actionsHistoryUNDO.size())
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
			switch((*it)->type)
			{
				case UNDOAction::ACTION_COLORIZE:
				case UNDOAction::ACTION_TILEMASK:
				{
					Image *image = (Image *)((*it)->actionData);
					SafeRelease(image);
					(*it)->actionData = NULL;
					break;
				}

				case UNDOAction::ACTION_VISIBILITY:
				{
					VisibilityActionDataStruct* data = (VisibilityActionDataStruct*)((*it)->actionData);
					Image* image = (Image*)(data->image);
					SafeRelease(image);
					SafeDelete(data);
					(*it)->actionData = NULL;
					break;
				}

				default:
					FileSystem::Instance()->DeleteFile((*it)->filePathname);
					break;
			}

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
		switch((*it)->type)
		{
			case UNDOAction::ACTION_COLORIZE:
			case UNDOAction::ACTION_TILEMASK:
			{
				Image *image = (Image *)((*it)->actionData);
				SafeRelease(image);
				(*it)->actionData = NULL;
				break;
			}

			case UNDOAction::ACTION_VISIBILITY:
			{
				VisibilityActionDataStruct* data = (VisibilityActionDataStruct*)((*it)->actionData);
				Image* image = (Image*)(data->image);
				SafeRelease(image);
				SafeDelete(data);
				(*it)->actionData = NULL;
				break;
			}

			default:
				FileSystem::Instance()->DeleteFile((*it)->filePathname);
				break;
		}

        SafeRelease(*it);
    }
    actionsHistory.clear();
}

