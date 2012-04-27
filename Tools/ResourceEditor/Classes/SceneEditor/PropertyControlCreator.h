#ifndef __PROPERTY_CONTROL_CREATOR_H__
#define __PROPERTY_CONTROL_CREATOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class NodesPropertyControl;
class PropertyControlCreator: public Singleton<PropertyControlCreator>
{
    enum ePropertyControlIDs
    {
        EPCID_LIGHT,
        EPCID_CUBE,
        EPCID_SPHERE,
        EPCID_CAMERA,
        EPCID_LANDSCAPE,
        EPCID_MESH,
        EPCID_LODNODE,
        EPCID_NODE,
        
        EPCID_DATANODE,
        EPCID_MATERIAL,
        
        EPCID_LANDSCAPE_EDITOR_MASK,
        EPCID_LANDSCAPE_EDITOR_HEIGHT,
        
        EPCID_COUNT
    };
    
public:
	
    PropertyControlCreator();
    virtual ~PropertyControlCreator();
    
    NodesPropertyControl * CreateControlForLandscapeEditor(SceneNode * sceneNode, const Rect & rect, LandscapeEditorPropertyControl::eEditorMode mode);
    NodesPropertyControl * CreateControlForNode(SceneNode * sceneNode, const Rect & rect, bool createNodeProperties);
	NodesPropertyControl * CreateControlForNode(DataNode * dataNode, const Rect & rect, bool createNodeProperties);


private:
    
    NodesPropertyControl * CreateControlForNode(ePropertyControlIDs controlID, const Rect & rect, bool createNodeProperties);

    
    NodesPropertyControl *controls[EPCID_COUNT];
    
};

#endif //__PROPERTY_CONTROL_CREATOR_H__
