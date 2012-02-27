#ifndef __TEXTURE_TRIANGLES_DIALOG_H__
#define __TEXTURE_TRIANGLES_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"
#include "ComboBox.h"

using namespace DAVA;

class TextureTrianglesDialog: public ExtendedDialog, public ComboBoxDelegate
{
public:
    TextureTrianglesDialog();
    virtual ~TextureTrianglesDialog();

    void Show(PolygonGroup *polygonGroup);
    
    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

    
protected:

    virtual const Rect DialogRect();
    virtual void Close();

    void OnClose(BaseObject * owner, void * userData, void * callerData);
    
    void InitWithData(PolygonGroup *pg);
    void FillRenderTarget(int32 textureIndex);

    void ConvertCoordinates(Vector2 &coordinates, const Vector2 &boxSize);

    
    ComboBox *combobox;
    PolygonGroup *workingPolygonGroup;
    
    UIControl *texturePreview;
    Sprite *previewSprite;
};



#endif // __SCENE_INFO_CONTROL_H__