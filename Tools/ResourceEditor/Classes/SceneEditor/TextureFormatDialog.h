#ifndef __TEXTURE_FORMAT_DIALOG_H__
#define __TEXTURE_FORMAT_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"

using namespace DAVA;

class TextureFormatDialogDelegate
{
public:
    virtual void OnFormatSelected(int32 newFormat, bool generateMimpaps) = 0;
};

class UICheckBox;
class TextureFormatDialog: public ExtendedDialog
{
public:
    TextureFormatDialog(TextureFormatDialogDelegate *newDelegate);
    virtual ~TextureFormatDialog();

    void Show();

protected:

    virtual const Rect DialogRect();

    UIButton *closeButtonTop;
    void OnCancel(BaseObject * owner, void * userData, void * callerData);

    UIButton *convertButton;
    void OnConvert(BaseObject * owner, void * userData, void * callerData);
    
    TextureFormatDialogDelegate *delegate;

    //PVR
    enum ePVRButtons
    {
        PVR_NONE = -1,
        PVRTC4 = 0,
        PVRTC2,
        
        PVR_COUNT
    };
    
    ePVRButtons currentPVRButton;
    UIButton *pvrButtons[PVR_COUNT];
    void OnPVRButton(BaseObject * owner, void * userData, void * callerData);
    
    //MIPMAPS
    UICheckBox *mipmapEnabled;
};



#endif // __TEXTURE_FORMAT_DIALOG_H__