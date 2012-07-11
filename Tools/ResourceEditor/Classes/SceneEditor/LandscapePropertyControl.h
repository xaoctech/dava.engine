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
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor);

protected:
    Vector<String> tiledModes;

    void CreateMaskTexture(const String &lightmapPath, const String &alphamaskPath);

    void SetLandscapeTexture(LandscapeNode::eTextureLevel level, const String &texturePathname);

    void GenerateFullTiledTexture(BaseObject * object, void * userData, void * callerData);
    
    void AddFilepathProperty(const String &key, const String &filter, LandscapeNode::eTextureLevel level);
};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
