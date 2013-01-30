#include "Scene3D/Components/RenderComponent.h"

namespace DAVA 
{

RenderComponent::RenderComponent(RenderObject * _object)
{
    renderObject = SafeRetain(_object);
}

RenderComponent::~RenderComponent()
{
    SafeRelease(renderObject);
}
    
void RenderComponent::SetRenderObject(RenderObject * _renderObject)
{
	SafeRelease(renderObject);
    renderObject = SafeRetain(_renderObject);
}
    
RenderObject * RenderComponent::GetRenderObject()
{
    return renderObject;
}
    
Component * RenderComponent::Clone(SceneNode * toEntity)
{
    RenderComponent * component = new RenderComponent();
	component->SetEntity(toEntity);

    component->renderObject = renderObject->Clone();
    return component;
}

void RenderComponent::GetDataNodes(Set<DAVA::DataNode *> &dataNodes)
{
    uint32 count = renderObject->GetRenderBatchCount();
    for(uint32 i = 0; i < count; ++i)
    {
        RenderBatch *renderBatch = renderObject->GetRenderBatch(i);

        Material *material = renderBatch->GetMaterial();
        if(material)
        {
			InsterDataNode(material, dataNodes);
        }
        
        PolygonGroup *pg = renderBatch->GetPolygonGroup();
        if(pg)
        {
			InsterDataNode(pg, dataNodes);
        }
    }
}

void RenderComponent::InsterDataNode(DataNode *node, Set<DataNode*> & dataNodes)
{
	dataNodes.insert(node);

	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		InsterDataNode(node->GetChild(i), dataNodes);
	}
}


};
