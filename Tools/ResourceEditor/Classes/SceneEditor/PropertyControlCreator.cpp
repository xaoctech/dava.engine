#include "PropertyControlCreator.h"
#include "NodesPropertyControl.h"
#include "LightPropertyControl.h"
#include "BoxPropertyControl.h"
#include "SpherePropertyControl.h"
#include "CameraPropertyControl.h"
#include "LandscapePropertyControl.h"
#include "LandscapeEditorPropertyControl.h"
#include "MaterialPropertyControl.h"
#include "LodNodePropertyControl.h"
#include "EntityPropertyControl.h"
#include "Entity/Entity.h"
#include "ParticleEmitterPropertyControl.h"


PropertyControlCreator::PropertyControlCreator()
{
    for(int32 iControl = 0; iControl < EPCID_COUNT; ++iControl)
    {
        controls[iControl] = NULL;
    }
}

PropertyControlCreator::~PropertyControlCreator()
{
    for(int32 iControl = 0; iControl < EPCID_COUNT; ++iControl)
    {
        SafeRelease(controls[iControl]);
    }
}


NodesPropertyControl * PropertyControlCreator::CreateControlForNode(SceneNode * sceneNode, const Rect & rect, bool createNodeProperties)
{
	LightNode * light = dynamic_cast<LightNode *>(sceneNode);
	if(light)
	{
        return CreateControlForNode(EPCID_LIGHT, rect, createNodeProperties);
	}
    
    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
    if(cube)
    {
        return CreateControlForNode(EPCID_CUBE, rect, createNodeProperties);
    }

    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
    if(sphere)
    {
        return CreateControlForNode(EPCID_SPHERE, rect, createNodeProperties);
    }

    Camera *camera = dynamic_cast<Camera *> (sceneNode);
    if(camera)
    {
        return CreateControlForNode(EPCID_CAMERA, rect, createNodeProperties);
    }

    LandscapeNode *landscape = dynamic_cast<LandscapeNode *> (sceneNode);
    if(landscape)
    {
        return CreateControlForNode(EPCID_LANDSCAPE, rect, createNodeProperties);
    }
    
    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *>(sceneNode); //must be later children of MeshInstanceNode
    if(mesh)
    {
        return CreateControlForNode(EPCID_MESH, rect, createNodeProperties);
    }
    
    LodNode *lodNode = dynamic_cast<LodNode*>(sceneNode);
    if(lodNode)
    {
        return CreateControlForNode(EPCID_LODNODE, rect, createNodeProperties);
    }

	ParticleEmitterNode * particleEmitterNode = dynamic_cast<ParticleEmitterNode *>(sceneNode);
	if(particleEmitterNode)
	{
		return CreateControlForNode(EPCID_PARTICLE_EMITTER, rect, createNodeProperties);
	}

	return CreateControlForNode(EPCID_NODE, rect, createNodeProperties);
}

NodesPropertyControl * PropertyControlCreator::CreateControlForNode(DataNode * dataNode, const Rect & rect, bool createNodeProperties)
{
    Material * material = dynamic_cast<Material *>(dataNode);
	if(material)
	{
        return CreateControlForNode(EPCID_MATERIAL, rect, createNodeProperties);
	}
    
	return CreateControlForNode(EPCID_DATANODE, rect, createNodeProperties);
}

NodesPropertyControl * PropertyControlCreator::CreateControlForNode(
                                                                    ePropertyControlIDs controlID, 
                                                                    const Rect & rect, bool createNodeProperties)
{
    if(controls[controlID] && (rect != controls[controlID]->GetRect()))
    {
        SafeRelease(controls[controlID]);
    }
 
    if(!controls[controlID])
    {
        switch (controlID) 
        {
            case EPCID_LIGHT:
                controls[controlID] = new LightPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_CUBE:
                controls[controlID] = new BoxPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_SPHERE:
                controls[controlID] = new SpherePropertyControl(rect, createNodeProperties);
                break;
            case EPCID_CAMERA:
                controls[controlID] = new CameraPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_LANDSCAPE:
                controls[controlID] = new LandscapePropertyControl(rect, createNodeProperties);
                break;
            case EPCID_MESH:
                controls[controlID] = new MeshInstancePropertyControl(rect, createNodeProperties);
                break;
            case EPCID_NODE:
                controls[controlID] = new NodesPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_LODNODE:
                controls[controlID] = new LodNodePropertyControl(rect, createNodeProperties);
                break;


            case EPCID_LANDSCAPE_EDITOR_MASK:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::MASK_EDITOR_MODE);
                break;

            case EPCID_LANDSCAPE_EDITOR_HEIGHT:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE);
                break;

            case EPCID_DATANODE:
                controls[controlID] = new NodesPropertyControl(rect, createNodeProperties);
                break;
            case EPCID_MATERIAL:
                controls[controlID] = new MaterialPropertyControl(rect, createNodeProperties);
				break;
			case EPCID_PARTICLE_EMITTER:
				controls[controlID] = new ParticleEmitterPropertyControl(rect, createNodeProperties);
				break;
                
            default:
                break; 
        }
    }

    
    return controls[controlID];
}

NodesPropertyControl * PropertyControlCreator::CreateControlForLandscapeEditor(SceneNode * sceneNode, const Rect & rect, LandscapeEditorPropertyControl::eEditorMode mode)
{
    if(LandscapeEditorPropertyControl::MASK_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_MASK, rect, false);
    }
    else if(LandscapeEditorPropertyControl::HEIGHT_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_HEIGHT, rect, false);
    }

    return NULL;
}

NodesPropertyControl * PropertyControlCreator::CreateControlForEntity(Entity * entity, const Rect & rect)
{
	if(controls[EPCID_ENTITY] && (rect != controls[EPCID_ENTITY]->GetRect()))
	{
		SafeRelease(controls[EPCID_ENTITY]);
	}

	if(!controls[EPCID_ENTITY])
	{
		controls[EPCID_ENTITY] = new EntityPropertyControl(rect, false);
	}

	return controls[EPCID_ENTITY];
}
