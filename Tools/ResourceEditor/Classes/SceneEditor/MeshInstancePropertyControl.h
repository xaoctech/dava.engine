#ifndef __MESH_INSTANCE_PROPERTY_CONTROL_H__
#define __MESH_INSTANCE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class MeshInstancePropertyControl : public NodesPropertyControl
{
public:
	MeshInstancePropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~MeshInstancePropertyControl();

	virtual void ReadFrom(SceneNode * sceneNode);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    
protected:

    int32 GetIndexFromKey(int32 pos, const String &forKey);
    void OnGo2Materials(BaseObject * object, void * userData, void * callerData);

    Vector<Material*> materials;
    Vector<String> materialNames;
};

#endif //__MESH_INSTANCE_PROPERTY_CONTROL_H__
