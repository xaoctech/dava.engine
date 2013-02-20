#include "PropertyControlCreator.h"
#include "NodesPropertyControl.h"
#include "LightPropertyControl.h"
#include "CameraPropertyControl.h"
#include "LandscapePropertyControl.h"
#include "LandscapeEditorPropertyControl.h"
#include "MaterialPropertyControl.h"
#include "LodNodePropertyControl.h"
#include "EntityPropertyControl.h"
#include "ParticleEmitterPropertyControl.h"
#include "SwitchNodePropertyControl.h"
#include "ParticleEffectPropertyControl.h"
#include "MeshInstancePropertyControl.h"


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
	return CreateControlForNode(DetectNodeType(sceneNode), rect, createNodeProperties);
    
}


PropertyControlCreator::ePropertyControlIDs PropertyControlCreator::DetectNodeType(SceneNode *node)
{
    if(node->GetComponent(Component::LIGHT_COMPONENT))
    {
        return EPCID_LIGHT;
    }
    
    if(node->GetComponent(Component::CAMERA_COMPONENT))
    {
        return EPCID_CAMERA;
    }
    
    if(node->GetComponent(Component::SWITCH_COMPONENT))
    {
        return EPCID_SWITCH;
    }
    
    if(GetEmitter(node))
    {
        return EPCID_PARTICLE_EMITTER;
    }
    
    if(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))
    {
        return EPCID_PARTICLE_EFFECT;
    }

    if(node->GetComponent(Component::LOD_COMPONENT))
    {
        return EPCID_LODNODE;
    }

    if(GetLandscape(node))
    {
        return EPCID_LANDSCAPE;
    }
    
    return EPCID_NODE;
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

			case EPCID_LANDSCAPE_EDITOR_COLORIZE:
                controls[controlID] = new LandscapeEditorPropertyControl(rect, createNodeProperties, LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE);
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
			case EPCID_SWITCH:
				controls[controlID] = new SwitchNodePropertyControl(rect, createNodeProperties);
				break;

			case EPCID_PARTICLE_EFFECT:
				controls[controlID] = new ParticleEffectPropertyControl(rect, createNodeProperties);
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
	else if(LandscapeEditorPropertyControl::COLORIZE_EDITOR_MODE == mode)
    {
        return CreateControlForNode(EPCID_LANDSCAPE_EDITOR_COLORIZE, rect, false);
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
