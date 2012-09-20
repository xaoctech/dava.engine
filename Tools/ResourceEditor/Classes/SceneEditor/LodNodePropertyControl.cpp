#include "LodNodePropertyControl.h"


LodNodePropertyControl::LodNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

LodNodePropertyControl::~LodNodePropertyControl()
{

}


void LodNodePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    LodNode *lodNode = dynamic_cast<LodNode*> (sceneNode);
	DVASSERT(lodNode);

    propertyList->AddSection("property.lodnode", GetHeaderState("property.lodnode", true));
    
    propertyList->AddBoolProperty("property.lodnode.forcedistance");
    propertyList->SetBoolPropertyValue("property.lodnode.forcedistance", false);
    propertyList->AddSliderProperty("property.lodnode.distanceslider", false);
    propertyList->SetSliderPropertyValue("property.lodnode.distanceslider", 0, LodNode::MAX_LOD_DISTANCE, LodNode::MIN_LOD_DISTANCE);

    int32 lodCount = lodNode->GetLodLayersCount();
    if(1 < lodCount)
    {
        propertyList->AddDistanceProperty("property.lodnode.distances");
        float32 *distances = new float32[lodNode->GetLodLayersCount()];
        int32 *triangles = new int32[lodNode->GetLodLayersCount()];
        
        
        List<LodNode::LodData*> lodLayers;
        lodNode->GetLodData(lodLayers);
        
        List<LodNode::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
        for(int32 i = 0; i < lodNode->GetLodLayersCount(); ++i)
        {
            distances[i] = lodNode->GetLodLayerDistance(i);
            
            LodNode::LodData *layer = *lodLayerIt;
            triangles[i] = GetTrianglesForLodLayer(layer);

            ++lodLayerIt;
        }
        
        propertyList->SetDistancePropertyValue("property.lodnode.distances", distances, triangles, lodNode->GetLodLayersCount());
        
        
        SafeDeleteArray(distances);
        SafeDeleteArray(triangles);
    }
}



