#include "LodNodePropertyControl.h"


LodNodePropertyControl::LodNodePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

LodNodePropertyControl::~LodNodePropertyControl()
{

}

//void LodNodePropertyControl::WillDisappear()
//{
//    LodNode *lodNode = dynamic_cast<LodNode *> (currentSceneNode);
//    if(lodNode)
//    {
//        lodNode->SetForceLodLayerDistance(LodNode::INVALID_DISTANCE);
//    }
//    
//    NodesPropertyControl::WillDisappear();
//}

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
        for(int32 i = 0; i < lodNode->GetLodLayersCount(); ++i)
        {
            distances[i] = lodNode->GetLodLayerDistance(i);
        }
        propertyList->SetDistancePropertyValue("property.lodnode.distances", distances, lodNode->GetLodLayersCount());
        SafeDeleteArray(distances);
    }
}

//void LodNodePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
//{
//    if("property.lodnode.forcedistance" == forKey)
//    {
//        LodNode *lodNode = dynamic_cast<LodNode *> (currentSceneNode);
//        
//        if(newValue)
//        {
//            lodNode->SetForceLodLayerDistance(propertyList->GetSliderPropertyValue("property.lodnode.distanceslider"));
//        }
//        else 
//        {
//            lodNode->SetForceLodLayerDistance(LodNode::INVALID_DISTANCE);
//        }
//    }
//
//    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
//}

//void LodNodePropertyControl::OnDistancePropertyChanged(PropertyList *forList, const String &forKey, float32 newValue, int32 index)
//{
//    if("property.lodnode.distances" == forKey)
//    {
//        LodNode *lodNode = dynamic_cast<LodNode *> (currentSceneNode);
//        lodNode->SetLodLayerDistance(index, newValue);
//    }
//    
//    NodesPropertyControl::OnDistancePropertyChanged(forList, forKey, newValue, index);
//}


//void LodNodePropertyControl::OnSliderPropertyChanged(PropertyList *forList, const String &forKey, float32 newValue)
//{
//    if("property.lodnode.distanceslider" == forKey)
//    {
//        if(propertyList->GetBoolPropertyValue("property.lodnode.forcedistance"))
//        {
//            LodNode *lodNode = dynamic_cast<LodNode *> (currentSceneNode);
//            lodNode->SetForceLodLayerDistance(newValue);
//        }
//    }
//
//    NodesPropertyControl::OnSliderPropertyChanged(forList, forKey, newValue);
//}

