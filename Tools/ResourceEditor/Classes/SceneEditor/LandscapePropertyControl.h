#ifndef __LANDSCAPE_PROPERTY_CONTROL_H__
#define __LANDSCAPE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class LandscapePropertyControl : public NodesPropertyControl
{
public:
	LandscapePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~LandscapePropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);

    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
	virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    
protected:
    Vector<String> renderingModes;

    void CreateMaskTexture(const String &lightmapPath, const String &alphamaskPath);

    void SetLandscapeTexture(LandscapeNode::eTextureLevel level, const String &texturePathname);

};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
