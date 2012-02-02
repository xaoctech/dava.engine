#ifndef __LAST_OPENED_FILES_DIALOG_H__
#define __LAST_OPENED_FILES_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"

using namespace DAVA;

class LastOpenedFilesDialogDelegate
{
public:
  
    virtual void OnLastFileSelected(String pathToFile) = 0;
};

class LastOpenedFilesDialog: public ExtendedDialog, public UIListDelegate
{
public:
    LastOpenedFilesDialog(LastOpenedFilesDialogDelegate *newDelegate);
    virtual ~LastOpenedFilesDialog();

    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    void Show();
    
protected:

    virtual const Rect DialogRect();
    void OnCancel(BaseObject * owner, void * userData, void * callerData);

    UIButton *closeButton;
    UIList *filesList;
    
    LastOpenedFilesDialogDelegate * delegate;
};



#endif // __LAST_OPENED_FILES_DIALOG_H__