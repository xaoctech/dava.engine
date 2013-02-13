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

    component->renderObject = renderObject->Clone(component->renderObject);
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
			InsertDataNode(material, dataNodes);
        }
        
        PolygonGroup *pg = renderBatch->GetPolygonGroup();
        if(pg)
        {
			InsertDataNode(pg, dataNodes);
        }
    }
}

void RenderComponent::InsertDataNode(DataNode *node, Set<DataNode*> & dataNodes)
{
	dataNodes.insert(node);

	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		InsertDataNode(node->GetChild(i), dataNodes);
	}
}

void RenderComponent::Serialize(KeyedArchive *archive)
{
	Component::Serialize(archive);

	if(NULL != archive && NULL != renderObject)
	{
		KeyedArchive *roArch = new KeyedArchive();
		renderObject->Serialize(roArch);

		archive->SetArchive("rc.renderObj", roArch);

		roArch->Release();
	}
}

void RenderComponent::Deserialize(KeyedArchive *archive)
{
	if(NULL != archive)
	{
		KeyedArchive *roArch = archive->GetArchive("rc.renderObj");
		if(NULL != roArch)
		{
			RenderObject* ro = new RenderObject();
			ro->Deserialize(roArch);

			SetRenderObject(ro);
		}
	}

	Component::Deserialize(archive);
}

};
