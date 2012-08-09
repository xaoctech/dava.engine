#ifndef __SCENE_PREVIEW_DIALOG_H__
#define __SCENE_PREVIEW_DIALOG_H__

#include "DAVAEngine.h"
#include "../EditorScene.h"
#include "../SceneEditor/ExtendedDialog.h"

using namespace DAVA;


class ScenePreviewControl;
class ScenePreviewDialog: public ExtendedDialog
{
    
public:
    ScenePreviewDialog();
    virtual ~ScenePreviewDialog();
    
    void Show(const String &scenePathname);
    
protected:

    void OnClose(BaseObject *, void *, void *);
    virtual const Rect DialogRect();

    ScenePreviewControl *preview;
    
    // general
    Font *fontLight;
    Font *fontDark;
    
    UIStaticText *errorMessage;
    
    
};



#endif // __SCENE_PREVIEW_DIALOG_H__