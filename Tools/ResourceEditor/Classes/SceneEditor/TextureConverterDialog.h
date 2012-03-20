#ifndef __TEXTURE_CONVERTER_DIALOG_H__
#define __TEXTURE_CONVERTER_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"
#include "TextureFormatDialog.h"

using namespace DAVA;

class UIZoomControl;
class TextureConverterDialog: public UIControl, public UIListDelegate, public TextureFormatDialogDelegate
{
public:
    TextureConverterDialog(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~TextureConverterDialog();

    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
    virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    virtual void Update(float32 timeElapsed);


    void Show(Scene * scene);
    virtual void OnFormatSelected(int32 newFormat, bool generateMimpaps);
    
protected:

    Scene *workingScene;
    
    void ReleaseTextures();
    
    void AddLine(const Rect &lineRect);

    UIButton *closeButtonTop;
    void OnCancel(BaseObject * owner, void * userData, void * callerData);
    
    void EnumerateTextures();
    void EnumerateTexturesFromMaterials();
    void EnumerateTexturesFromNodes(SceneNode * node);
    void CollectTexture(Texture *texture);

    void RestoreTextures(Texture *t, const String &newTexturePath);
    void RestoreTexturesFromMaterials(Texture *t, const String &newTexturePath);
    void RestoreTexturesFromNodes(Texture *t, const String &newTexturePath, SceneNode * node);
    
    
    Texture *GetTextureForIndex(int32 index);
    Texture *CreateFromImage(const String &fileName);
    
    UIList *textureList;
    int32 selectedItem;
    Set<Texture *> textures;
    
    UIButton *convertButton;
    void OnConvert(BaseObject * owner, void * userData, void * callerData);
    
    void SetupTexturePreview();
    UIZoomControl *lastActiveZoomControl;
    UIZoomControl *srcZoomPreview;
    UIZoomControl *dstZoomPreview;
    UIControl *srcPreview;
    UIControl *dstPreview;
    
    TextureFormatDialog *formatDialog;
    
    UISlider *zoomSlider;
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);
    
    Vector2 srcOffsetPrev;
    Vector2 dstOffsetPrev;
    
    void SetupZoomedPreview(Texture *tex, UIControl *preview, UIZoomControl *zoomControl);
    
    static String GetWorkingTexturePath(const String &relativeTexturePath);
    static String GetSrcTexturePath(const String &relativeTexturePath);

    static String NormalizePath(const String &pathname);

    String selectedTextureName;
};



#endif // __TEXTURE_CONVERTER_DIALOG_H__