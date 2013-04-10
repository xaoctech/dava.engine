//
//  EditorListDelegate.h
//  UIEditor
//
//  Created by Denis Bespalov on 4/5/13.
//
//

#ifndef __UIEditor__EditorListDelegate__
#define __UIEditor__EditorListDelegate__

#include "Base/BaseTypes.h"
#include "UI/UIList.h"

namespace DAVA {

class EditorListDelegate : public UIControl, public UIListDelegate
{

public:
	EditorListDelegate(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~EditorListDelegate();

	// UIListDelegate
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellHeight(UIList *forList, int32 index);
	//virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
	virtual void SaveToYaml(UIList *forList, YamlNode *node);
	
	void SetAggregatorID(int32 id);
	int32 GetAggregatorID();

private:
	int32 aggregatorID;
	Vector2 cellSize;
};

};

#endif /* defined(__UIEditor__EditorListDelegate__) */
