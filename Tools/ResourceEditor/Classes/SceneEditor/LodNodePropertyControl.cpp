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

    LodComponent *lodComponent = GetLodComponent(sceneNode);
	DVASSERT(lodComponent);

    propertyList->AddSection("property.lodnode", GetHeaderState("property.lodnode", true));
    
    propertyList->AddBoolProperty("property.lodnode.forcedistance");
    propertyList->SetBoolPropertyValue("property.lodnode.forcedistance", false);
    propertyList->AddSliderProperty("property.lodnode.distanceslider", false);
    propertyList->SetSliderPropertyValue("property.lodnode.distanceslider", 0, LodComponent::MAX_LOD_DISTANCE, LodComponent::MIN_LOD_DISTANCE);

    int32 lodCount = lodComponent->GetLodLayersCount();
    if(1 < lodCount)
    {
        propertyList->AddDistanceProperty("property.lodnode.distances");
        float32 *distances = new float32[lodComponent->GetLodLayersCount()];
        int32 *triangles = new int32[lodComponent->GetLodLayersCount()];
        
        
        Vector<LodComponent::LodData*> lodLayers;
        lodComponent->GetLodData(lodLayers);
        
        Vector<LodComponent::LodData*>::const_iterator lodLayerIt = lodLayers.begin();
        for(int32 i = 0; i < lodComponent->GetLodLayersCount(); ++i)
        {
            distances[i] = lodComponent->GetLodLayerDistance(i);
            
            LodComponent::LodData *layer = *lodLayerIt;
            triangles[i] = GetTrianglesForLodLayer(layer);

            ++lodLayerIt;
        }
        
        propertyList->SetDistancePropertyValue("property.lodnode.distances", distances, triangles, lodComponent->GetLodLayersCount());
        
        
        SafeDeleteArray(distances);
        SafeDeleteArray(triangles);
    }
}



