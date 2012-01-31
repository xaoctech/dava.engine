/*
 *  MaterialEditor.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "PropertyList.h"
#include "ComboBox.h"

using namespace DAVA;

class ComboBox;
class MaterialEditor : public DraggableDialog, public UIListDelegate, public ComboBoxDelegate, public PropertyListDelegate
{
public:
    
    enum eTextureType 
    {
        ME_DIFFUSE = 0,
        ME_DECAL,
        ME_DETAIL,
        ME_NORMAL_MAP,
        
        ME_TEX_COUNT
    };
    
    MaterialEditor();
    ~MaterialEditor();
    
    void SetWorkingScene(Scene *newWorkingScene, SceneNode *selectedSceneNode);
    void EditMaterial(Scene *newWorkingScene, Material *material);
    
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
    virtual void WillAppear();
    virtual void WillDisappear();
    virtual void UpdateInternalMaterialsVector();

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

	virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
	virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
	virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
	virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
	virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor);

    void SelectMaterial(int materialIndex);
    void PreparePropertiesForMaterialType(int materialType);
    
protected:
    Vector<Material*> materials;
    
    UIList *materialsList;
    ComboBox *materialTypes;
    PropertyList *materialProps[Material::MATERIAL_TYPES_COUNT];
    
    Scene *workingScene;
    SceneNode *workingSceneNode;
    Vector<Material*> workingNodeMaterials;
    void EnumerateNodeMaterials(SceneNode *node);
    
    int selectedMaterial;
    UIListCell *lastSelection;
    
    
    //===============
    enum eDisplayMode
    {
        EDM_ALL = 0,
        EDM_SELECTED
    };
    eDisplayMode displayMode;
    
    UIButton *btnAll;
    UIButton *btnSelected;
    
    void OnAllPressed(BaseObject * object, void * userData, void * callerData);
    void OnSelectedPressed(BaseObject * object, void * userData, void * callerData);
    
    void UdpateButtons(bool showButtons);
    void RefreshList();
    Material *GetMaterial(int32 index);
};

#endif