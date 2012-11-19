#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class UNDOAction: public BaseObject
{
public:
    enum eActionType
    {
        ACTION_NONE = -1,
        ACTION_HEIGHTMAP = 0,
        ACTION_TILEMASK,
		ACTION_COLORIZE,
		ACTION_VISIBILITY
    };
    
    UNDOAction();

    eActionType type;
    int32 ID;
    String filePathname;
    
    void *actionData;
};


class UNDOManager: public Singleton<UNDOManager>
{    
    enum eConst
    {
        UNDO_HISTORY_SIZE = 20
    };
    
public:
 
    UNDOManager();
	virtual ~UNDOManager();

    void SaveHightmap(Heightmap *heightmap);
    void UndoHeightmap(Heightmap *heightmap);
    void RedoHeightmap(Heightmap *heightmap);

    void SaveTilemask(Texture *tilemask);
    Texture * UndoTilemask();
    Texture * RedoTilemask();

	void SaveColorize(Texture *colTex);
    Texture * UndoColorize();
    Texture * RedoColorize();

	void SaveVisibility(Texture* visibilityAreaTexture, bool visibilityPointSet, const Vector2& visibilityPoint);
	void UndoVisibility(Texture** visibilityAreaTexture, bool* visibilityPointSet, Vector2* visibilityPoint);
	void RedoVisibility(Texture** visibilityAreaTexture, bool* visibilityPointSet, Vector2* visibilityPoint);


    UNDOAction::eActionType GetLastUNDOAction();
    UNDOAction::eActionType GetFirstREDOAction();
    
    void ClearHistory(UNDOAction::eActionType forAction);

protected:

    void ClearHistory(List<UNDOAction *> &actionsHistory, UNDOAction::eActionType forAction);
    void ReleaseHistory(List<UNDOAction *> &actionsHistory);
    
    UNDOAction *CreateHeightmapAction(Heightmap *heightmap);
    UNDOAction *CreateTilemaskAction(Texture *tilemask);
    UNDOAction *CreateColorizeAction(Texture *tilemask);
	UNDOAction* CreateVisibilityAction(Texture *visibilityAreaTexture, bool visibilityPointSet, const Vector2 &visibilityPoint);
    
    String TimeString();
    
    void CheckHistoryLength();

	Texture * UndoTexture();
    Texture * RedoTexture();
	void SaveTexture(Texture *tex, UNDOAction::eActionType type);
	UNDOAction * CreateTextureAction(Texture *tex, UNDOAction::eActionType type);
    
    List<UNDOAction *>actionsHistoryUNDO;
    List<UNDOAction *>actionsHistoryREDO;
    
    int32 actionCounter;
};



#endif // __UNDO_MANAGER_H__