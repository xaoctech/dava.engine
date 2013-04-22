#ifndef __MATERIAL_PROPERTY_CONTROL_H__
#define __MATERIAL_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class MaterialPropertyControl : public NodesPropertyControl
{
    enum eTextureType 
    {
        ETT_DIFFUSE = 0,
        ETT_DECAL,
        ETT_DETAIL,
        ETT_NORMAL_MAP,
        
        ME_TEX_COUNT
    };

    
public:
	MaterialPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~MaterialPropertyControl();

    virtual void ReadFrom(DataNode *dataNode);

    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue);
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    
protected:
    
    void SetFilepathValue(Material *material, int32 type);
    
};

#endif //__MATERIAL_PROPERTY_CONTROL_H__
