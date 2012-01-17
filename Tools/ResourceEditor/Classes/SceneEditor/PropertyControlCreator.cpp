#include "PropertyControlCreator.h"
#include "NodesPropertyControl.h"
#include "LightPropertyControl.h"
#include "BoxPropertyControl.h"
#include "SpherePropertyControl.h"
#include "CameraPropertyControl.h"
#include "LandscapePropertyControl.h"

NodesPropertyControl * PropertyControlCreator::CreateControlForNode(SceneNode * sceneNode, const Rect & rect, bool createNodeProperties)
{
	LightNode * light = dynamic_cast<LightNode *>(sceneNode);
	if(light)
	{
		return new LightPropertyControl(rect, createNodeProperties);
	}
    
    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
    if(cube)
    {
        return  new BoxPropertyControl(rect, createNodeProperties);
    }

    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
    if(sphere)
    {
        return  new SpherePropertyControl(rect, createNodeProperties);
    }

    Camera *camera = dynamic_cast<Camera *> (sceneNode);
    if(camera)
    {
        return  new CameraPropertyControl(rect, createNodeProperties);
    }

    LandscapeNode *landscape = dynamic_cast<LandscapeNode *> (sceneNode);
    if(landscape)
    {
        return  new LandscapePropertyControl(rect, createNodeProperties);
    }
    
    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *>(sceneNode); //must be later children of MeshInstanceNode
    if(mesh)
    {
        return  new MeshInstancePropertyControl(rect, createNodeProperties);
    }
    
	return new NodesPropertyControl(rect, createNodeProperties);
}
