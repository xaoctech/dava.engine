#include "MeshInstancePropertyControl.h"

#include "SceneEditorScreenMain.h"
#include "../AppScreens.h"

MeshInstancePropertyControl::MeshInstancePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

MeshInstancePropertyControl::~MeshInstancePropertyControl()
{

}

void MeshInstancePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (sceneNode);
	DVASSERT(mesh);

    propertyList->AddSection("property.meshinstance.meshinstance", GetHeaderState("property.meshinstance.meshinstance", true));
        
    //BBOX
    AABBox3 bbox = mesh->GetBoundingBox();
    AABBox3 transformedBox;
    bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
    
    propertyList->AddStringProperty("property.meshinstance.bboxmin", PropertyList::PROPERTY_IS_READ_ONLY);
    propertyList->AddStringProperty("property.meshinstance.bboxmax", PropertyList::PROPERTY_IS_READ_ONLY);
    
    propertyList->SetStringPropertyValue("property.meshinstance.bboxmin", Format("%0.2f, %0.2f, %0.2f", 
                                                            transformedBox.min.x, transformedBox.min.y, transformedBox.min.z));
    propertyList->SetStringPropertyValue("property.meshinstance.bboxmax", Format("%0.2f, %0.2f, %0.2f", 
                                                            transformedBox.max.x, transformedBox.max.y, transformedBox.max.z));
    
    materials.clear();
    materialNames.clear();
    
    //Vector<Material*> materials;
    workingScene->GetDataNodes(materials);
    int32 matCount = (int32)materials.size();
    for(int32 i = 0; i < materials.size(); ++i)
    {
        Material *mat = materials[i];
        materialNames.push_back(mat->GetName());
    }
    
    
    Vector<int32> groupIndexes = mesh->GetPolygonGroupIndexes();
    Vector<Material*> meshMaterials = mesh->GetMaterials();
    Vector<StaticMesh*> meshes = mesh->GetMeshes();
    
    for(int32 i = 0; i < meshes.size(); ++i)
    {
        PolygonGroup *pg = meshes[i]->GetPolygonGroup(groupIndexes[i]);
        
        String fieldName = Format("PolygonGroup #%d", i);
        propertyList->AddSection(fieldName, GetHeaderState(fieldName, true));
        
        int32 vertexFormat = pg->GetFormat();
        
        String keyPrefix = Format("#%d", i);
        propertyList->AddBoolProperty(keyPrefix + ". fmt.NORMAL", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.NORMAL", vertexFormat & EVF_NORMAL);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.COLOR", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.COLOR", vertexFormat & EVF_COLOR);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD0", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD0", vertexFormat & EVF_TEXCOORD0);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD1", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD1", vertexFormat & EVF_TEXCOORD1);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD2", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD2", vertexFormat & EVF_TEXCOORD2);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TEXCOORD3", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD3", vertexFormat & EVF_TEXCOORD3);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.TANGENT", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.TANGENT", vertexFormat & EVF_TANGENT);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.BINORMAL", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.BINORMAL", vertexFormat & EVF_BINORMAL);
        
        propertyList->AddBoolProperty(keyPrefix + ". fmt.JOINTWEIGHT", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->SetBoolPropertyValue(keyPrefix + ". fmt.JOINTWEIGHT", vertexFormat & EVF_JOINTWEIGHT);

		propertyList->AddIntProperty(keyPrefix + ".lightmap.size");
		propertyList->SetIntPropertyValue(keyPrefix + ".lightmap.size", currentNode->GetCustomProperties()->GetInt32(keyPrefix + ".lightmap.size", 128));
        
        if(matCount && !createNodeProperties)
        {
            String comboName = keyPrefix + ". Material";
            propertyList->AddComboProperty(comboName, materialNames);
            
            if(meshMaterials[i])
            {
                String meshMatName = meshMaterials[i]->GetName();
                for(int32 iMat = 0; iMat < materials.size(); ++iMat)
                {
                    if(meshMatName == materialNames[iMat])
                    {
                        propertyList->SetComboPropertyIndex(comboName, iMat);
                        break;
                    }
                }
            }
            else
            {
                propertyList->SetComboPropertyIndex(comboName, 0);
            }
            
            propertyList->AddMessageProperty("property.meshinstance.editmaterial", 
                                             Message(this, &MeshInstancePropertyControl::OnGo2Materials, meshMaterials[i]));
        }
    }

	propertyList->AddSection("property.meshinstance.dynamicshadow", GetHeaderState("property.meshinstance.dynamicshadow", true));
	
	propertyList->AddBoolProperty("property.meshinstance.dynamicshadow.enable");
	propertyList->SetBoolPropertyValue("property.meshinstance.dynamicshadow.enable", currentNode->GetCustomProperties()->GetBool("property.meshinstance.dynamicshadow.enable", false));
}


void MeshInstancePropertyControl::OnGo2Materials(DAVA::BaseObject *object, void *userData, void *callerData)
{
    Material *material = (Material *)userData;
    SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
    screen->EditMaterial(material);
}

void MeshInstancePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(forKey[0] == '#')
    {
        int32 index = GetIndexFromKey(forKey);
        
        String keyPrefix = Format("#%d", index);
        int32 vertexFormat = EVF_VERTEX;
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.NORMAL");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.COLOR");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD0");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD1");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD2");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.TEXCOORD3");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.TANGENT");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.BINORMAL");
        vertexFormat |= propertyList->GetBoolPropertyValue(keyPrefix + ". fmt.JOINTWEIGHT");
    }

	if(forKey == "property.meshinstance.dynamicshadow.enable")
	{
		currentNode->GetCustomProperties()->SetBool(forKey, newValue);
		if(newValue)
		{
			((MeshInstanceNode*)currentNode)->CreateDynamicShadowNode();
		}
		else
		{
			((MeshInstanceNode*)currentNode)->DeleteDynamicShadowNode();
		}
	}

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void MeshInstancePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if(forKey[0] == '#')
	{
		int32 index = GetIndexFromKey(forKey);

		String keyPrefix = Format("#%d", index);
		currentNode->GetCustomProperties()->SetInt32(keyPrefix + ".lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void MeshInstancePropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, 
                                               int32 newItemIndex, const String &newItemKey)
{
	if(forKey[0] == '#')
    {
        int32 index = GetIndexFromKey(forKey);

        MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (currentNode);
        mesh->ReplaceMaterial(materials[newItemIndex], index);
        
        UpdateFieldsForCurrentNode();
    }

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

int32 MeshInstancePropertyControl::GetIndexFromKey(const String &forKey)
{
	String num = forKey.substr(1);
    int32 retNum = atoi(num.c_str());
    
    return retNum;
}
