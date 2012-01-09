#include "NodesPropertyControl.h"
#include "ControlsFactory.h"


NodesPropertyControl::NodesPropertyControl(const Rect & rect, bool _createNodeProperties)
    :   UIControl(rect)
{
    deletionList = NULL;
    listHolder = NULL;
    btnCancel = NULL;
    
    
    nodesDelegate = NULL;
    currentNode = NULL;
    createNodeProperties = _createNodeProperties;
    projectPath = "/";
    
    types.push_back("Directional");
    types.push_back("Spot");
    types.push_back("Point");

    renderingModes.push_back("TEXTURE");
    renderingModes.push_back("SHADER");
    renderingModes.push_back("BLENDED_SHADER");
    
    materialTypes.push_back("UNLIT");
    materialTypes.push_back("UNLIT_DETAIL");
    materialTypes.push_back("UNLIT_DECAL");
    materialTypes.push_back("VERTEX_LIT");
    materialTypes.push_back("VERTEX_LIT_DETAIL");
    materialTypes.push_back("VERTEX_LIT_DECAL");
    materialTypes.push_back("NORMAL_MAPPED_DIFFUSE");
    materialTypes.push_back("NORMAL_MAPPED_SPECULAR");

    Rect propertyRect(0, 0, rect.dx, rect.dy);
    
    if(!createNodeProperties)
    {
        propertyRect.dy -= ControlsFactory::BUTTON_HEIGHT;
        
        btnPlus = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy, 
                                                     ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                L"+");
        btnPlus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnPlus));
        AddControl(btnPlus);
        
        btnMinus = ControlsFactory::CreateButton(
                                                 Rect(ControlsFactory::BUTTON_HEIGHT, propertyRect.dy, 
                                                      ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                                                 L"-");
        btnMinus->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnMinus));
        AddControl(btnMinus);
        
        propControl = new CreatePropertyControl(Rect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT*4, 
                                                     rect.dx, ControlsFactory::BUTTON_HEIGHT*3), this);
        
        
        listHolder = new UIControl(propertyRect);
        btnCancel = ControlsFactory::CreateButton(
                                                Rect(0, propertyRect.dy - ControlsFactory::BUTTON_HEIGHT, 
                                                propertyRect.dx, ControlsFactory::BUTTON_HEIGHT), 
                                                L"Cancel");
        btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &NodesPropertyControl::OnCancel));
        listHolder->AddControl(btnCancel);
    }
    
    propertyList = new PropertyList(propertyRect, this);
    AddControl(propertyList);
}
    
NodesPropertyControl::~NodesPropertyControl()
{
    SafeRelease(deletionList);
    SafeRelease(listHolder);
    SafeRelease(btnCancel);

    
    SafeRelease(propControl);
    
    SafeRelease(btnMinus);
    SafeRelease(btnPlus);
    
    
    SafeRelease(propertyList);
}

void NodesPropertyControl::WillAppear()
{
}

void NodesPropertyControl::ReadFrom(SceneNode *sceneNode)
{
    currentNode = sceneNode;
    propertyList->ReleaseProperties();
    
    UpdateProjectPath();

    if(!createNodeProperties)
    {
        propertyList->AddSection("General C++");
        propertyList->AddIntProperty("Retain Count", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("Class Name", PropertyList::PROPERTY_IS_READ_ONLY);
        propertyList->AddStringProperty("C++ Class Name", PropertyList::PROPERTY_IS_READ_ONLY);
        
        propertyList->SetIntPropertyValue("Retain Count", sceneNode->GetRetainCount());
        propertyList->SetStringPropertyValue("Class Name", sceneNode->GetClassName());
        propertyList->SetStringPropertyValue("C++ Class Name", typeid(*sceneNode).name());
    }

    propertyList->AddSection("Scene Node");
    propertyList->AddStringProperty("Name", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetStringPropertyValue("Name", sceneNode->GetName());

    if(!createNodeProperties)
    {
        propertyList->AddSection("Matrixes", false);
        
        propertyList->AddMatrix4Property("Local Matrix", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddMatrix4Property("World Matrix", PropertyList::PROPERTY_IS_READ_ONLY);

        propertyList->SetMatrix4PropertyValue("Local Matrix", sceneNode->GetLocalTransform());
        propertyList->SetMatrix4PropertyValue("World Matrix", sceneNode->GetWorldTransform());
    }
    
    Camera *camera = dynamic_cast<Camera*> (sceneNode);
    if(camera)
    {
        propertyList->AddSection("Camera");
        
        propertyList->AddFloatProperty("Fov", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("zNear", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("zFar", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddBoolProperty("isOrtho", PropertyList::PROPERTY_IS_EDITABLE);
        
        propertyList->AddFloatProperty("position.x", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("position.y", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("position.z", PropertyList::PROPERTY_IS_EDITABLE);
        
        propertyList->AddFloatProperty("target.x", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("target.y", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("target.z", PropertyList::PROPERTY_IS_EDITABLE);
        
        propertyList->SetFloatPropertyValue("Fov", camera->GetFOV());
        propertyList->SetFloatPropertyValue("zNear", camera->GetZNear());
        propertyList->SetFloatPropertyValue("zFar", camera->GetZFar());
        propertyList->SetBoolPropertyValue("isOrtho", camera->GetIsOrtho());
        
        Vector3 pos = camera->GetPosition();
        propertyList->SetFloatPropertyValue("position.x", pos.x);
        propertyList->SetFloatPropertyValue("position.y", pos.y);
        propertyList->SetFloatPropertyValue("position.z", pos.z);
        
        Vector3 target = camera->GetTarget();
        propertyList->SetFloatPropertyValue("target.x", target.x);
        propertyList->SetFloatPropertyValue("target.y", target.y);
        propertyList->SetFloatPropertyValue("target.z", target.z);
    }
    
    LightNode *light = dynamic_cast<LightNode *> (sceneNode);
    if(light)
    {
        propertyList->AddSection("Light");
        
        propertyList->AddComboProperty("Type", types);
        propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
        propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

        propertyList->SetComboPropertyIndex("Type", light->GetType());
        propertyList->SetFloatPropertyValue("r", light->GetColor().r);
        propertyList->SetFloatPropertyValue("g", light->GetColor().g);
        propertyList->SetFloatPropertyValue("b", light->GetColor().b);
        propertyList->SetFloatPropertyValue("a", light->GetColor().a);
    }
    
    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (sceneNode);
    if(mesh)
    {
        propertyList->AddSection("Mesh Instance");
        
        
        Vector<int32> groupIndexes = mesh->GetPolygonGroupIndexes();
        Vector<Material*> materials = mesh->GetMaterials();
        Vector<StaticMesh*> meshes = mesh->GetMeshes();

        for(int32 i = 0; i < meshes.size(); ++i)
        {
//            PolygonGroup *pg = meshes[i]->GetPolygonGroup(groupIndexes[i]);
//            
            String fieldName = Format("PolygonGroup #%d", i);
            propertyList->AddSection(fieldName);
            
            if(materials[i])
            {
                propertyList->AddComboProperty("Material", materialTypes);
                propertyList->SetComboPropertyIndex("Material", materials[i]->type);
            }
        }
    }
    
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
    if(landscape)
    {
        propertyList->AddSection("Landscape");
        
        propertyList->AddFloatProperty("Size", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("Height", PropertyList::PROPERTY_IS_EDITABLE); 
        
        propertyList->AddComboProperty("renderingMode", renderingModes);
        
        propertyList->AddFilepathProperty("HeightMap", ".png", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", ".png", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", ".png", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFilepathProperty("TEXTURE_BUMP", ".png", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", ".png", PropertyList::PROPERTY_IS_EDITABLE);

        
        AABBox3 bbox = landscape->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
        
        Vector3 size = transformedBox.max - transformedBox.min;
        propertyList->SetFloatPropertyValue("Size", size.x);
        propertyList->SetFloatPropertyValue("Height", size.z);
        
        propertyList->SetComboPropertyIndex("renderingMode", landscape->GetRenderingMode());
        
        String heightMap = landscape->GetHeightMapPathname();
        if(heightMap.length())
        {
            propertyList->SetFilepathPropertyValue("HeightMap", heightMap);
        }
        else
        {
            propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
        }
        
        Texture *t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE0);
        if(t)
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", t->GetPathname());
        }
        else
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
        }
        
        t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE1);
        if(t)
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", t->GetPathname());
        }
        else
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
        }
        
        t = landscape->GetTexture(LandscapeNode::TEXTURE_BUMP);
        if(t)
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", t->GetPathname());
        }
        else
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
        }
        
        t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTUREMASK);
        if(t)
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK",t->GetPathname());
        }
        else
        {
            propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
        }
    }
    
    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
    if (cube)
    {
        propertyList->AddSection("Cube");

        propertyList->AddFloatProperty("Length", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("Width", PropertyList::PROPERTY_IS_EDITABLE); 
        propertyList->AddFloatProperty("Depth", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
        propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

        Vector3 size = cube->GetSize();
        propertyList->SetFloatPropertyValue("Length", size.x);
        propertyList->SetFloatPropertyValue("Width", size.y); 
        propertyList->SetFloatPropertyValue("Depth", size.z);
        propertyList->SetFloatPropertyValue("r", cube->GetColor().r);
        propertyList->SetFloatPropertyValue("g", cube->GetColor().g);
        propertyList->SetFloatPropertyValue("b", cube->GetColor().b);
        propertyList->SetFloatPropertyValue("a", cube->GetColor().a);
    }
    
    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
    if(sphere)
    {
        propertyList->AddSection("Sphere");

        propertyList->AddFloatProperty("Radius", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("r", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("g", PropertyList::PROPERTY_IS_EDITABLE);
        propertyList->AddFloatProperty("b", PropertyList::PROPERTY_IS_EDITABLE); 
        propertyList->AddFloatProperty("a", PropertyList::PROPERTY_IS_EDITABLE); 

        float32 radius = sphere->GetRadius();
        propertyList->SetFloatPropertyValue("Radius", radius);
        propertyList->SetFloatPropertyValue("r", sphere->GetColor().r);
        propertyList->SetFloatPropertyValue("g", sphere->GetColor().g);
        propertyList->SetFloatPropertyValue("b", sphere->GetColor().b);
        propertyList->SetFloatPropertyValue("a", sphere->GetColor().a);
    }
    
    //must be last
    if(!createNodeProperties)
    {
        propertyList->AddSection("Custom properties");
        
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it)
        {
            String name = it->first;
            VariantType key = it->second;
            switch (key.type) 
            {
                case VariantType::TYPE_BOOLEAN:
                    propertyList->AddBoolProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetBoolPropertyValue(name, key.AsBool());
                    break;
                    
                case VariantType::TYPE_STRING:
                    propertyList->AddStringProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetStringPropertyValue(name, key.AsString());
                    break;

                case VariantType::TYPE_INT32:
                    propertyList->AddIntProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetIntPropertyValue(name, key.AsInt32());
                    break;

                case VariantType::TYPE_FLOAT:
                    propertyList->AddFloatProperty(name, PropertyList::PROPERTY_IS_EDITABLE);
                    propertyList->SetFloatPropertyValue(name, key.AsFloat());
                    break;
                    
                default:
                    break;
            }
        }
    }
}

void NodesPropertyControl::WriteTo(SceneNode *sceneNode)
{
    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
        
    if(!createNodeProperties)
    {
        sceneNode->SetLocalTransform(propertyList->GetMatrix4PropertyValue("Local Matrix"));
        //    sceneNode->SetWordTransform(propertyList->GetMatrix4PropertyValue("World Matrix"));
    }
    
    Camera *camera = dynamic_cast<Camera*> (sceneNode);
    if(camera)
    {
        camera->Setup(
                      propertyList->GetFloatPropertyValue("Fov"),
                      320.0f / 480.0f,
                      propertyList->GetFloatPropertyValue("zNear"),
                      propertyList->GetFloatPropertyValue("zFar"),
                      propertyList->GetBoolPropertyValue("isOrtho"));
        
        camera->SetPosition(Vector3(
                                    propertyList->GetFloatPropertyValue("position.x"),
                                    propertyList->GetFloatPropertyValue("position.y"),
                                    propertyList->GetFloatPropertyValue("position.z")));
        camera->SetTarget(Vector3(
                                  propertyList->GetFloatPropertyValue("target.x"),
                                  propertyList->GetFloatPropertyValue("target.y"),
                                  propertyList->GetFloatPropertyValue("target.z")));
    }
    
    LightNode *light = dynamic_cast<LightNode *> (sceneNode);
    if(light)
    {
        Color color(
                    propertyList->GetFloatPropertyValue("r"),
                    propertyList->GetFloatPropertyValue("g"),
                    propertyList->GetFloatPropertyValue("b"),
                    propertyList->GetFloatPropertyValue("a"));
        
        int32 type = propertyList->GetComboPropertyIndex("Type");
        
        light->SetColor(color);
        light->SetType((LightNode::eType)type);
    }
    
    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *> (sceneNode);
    if(mesh)
    {
        //Add Code
    }
    
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
    if(landscape)
    {
        Vector3 size(
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        

        int32 renderingMode = propertyList->GetComboPropertyIndex("renderingMode");

        String heightMap = propertyList->GetFilepathPropertyValue("HeightMap");
        String texture0 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE0");
        String texture1 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL");
        String textureBump = propertyList->GetFilepathPropertyValue("TEXTURE_BUMP");
        String textureUnmask = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTUREMASK");
        
        if(IsValidPath(heightMap))
        {
            landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
        }
        
        Texture::EnableMipmapGeneration();
        if(IsValidPath(texture0))
        {
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, texture0);
        }
        
        if(IsValidPath(texture1))
        {
            landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, texture1);
        }
        
        if(IsValidPath(textureBump))
        {
            landscape->SetTexture(LandscapeNode::TEXTURE_BUMP, textureBump);
        }
        
        if(IsValidPath(textureUnmask))
        {
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, textureUnmask);
        }
        Texture::DisableMipmapGeneration();
    }
    
    CubeNode *cube = dynamic_cast<CubeNode *> (sceneNode);
    if (cube)
    {
        Color color(
                    propertyList->GetFloatPropertyValue("r"),
                    propertyList->GetFloatPropertyValue("g"),
                    propertyList->GetFloatPropertyValue("b"),
                    propertyList->GetFloatPropertyValue("a"));
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("Length"),
                     propertyList->GetFloatPropertyValue("Width"),
                     propertyList->GetFloatPropertyValue("Depth"));
        
        cube->SetSize(size);
        cube->SetColor(color);
    }
    
    SphereNode *sphere = dynamic_cast<SphereNode *> (sceneNode);
    if(sphere)
    {
        Color color(
                    propertyList->GetFloatPropertyValue("r"),
                    propertyList->GetFloatPropertyValue("g"),
                    propertyList->GetFloatPropertyValue("b"),
                    propertyList->GetFloatPropertyValue("a"));
        
        float32 radius = propertyList->GetFloatPropertyValue("Radius");
        
        sphere->SetColor(color);
        sphere->SetRadius(radius);
    }
    
    //must be last
    if(!createNodeProperties)
    {
        KeyedArchive *customProperties = sceneNode->GetCustomProperties();
        Map<String, VariantType> propsData = customProperties->GetArchieveData();
        for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it)
        {
            String name = it->first;
            VariantType key = it->second;
            switch (key.type) 
            {
                case VariantType::TYPE_BOOLEAN:
                    customProperties->SetBool(name, propertyList->GetBoolPropertyValue(name));
                    break;
                    
                case VariantType::TYPE_STRING:
                    customProperties->SetString(name, propertyList->GetStringPropertyValue(name));
                    break;
                    
                case VariantType::TYPE_INT32:
                    customProperties->SetInt32(name, propertyList->GetIntPropertyValue(name));
                    break;
                    
                case VariantType::TYPE_FLOAT:
                    customProperties->SetFloat(name, propertyList->GetFloatPropertyValue(name));
                    break;
                    
                default:
                    break;
            }
        }
    }
}

bool NodesPropertyControl::IsValidPath(const String &path)
{
    size_t pos = path.find(".png");
    return (String::npos != pos);
}

void NodesPropertyControl::SetDelegate(NodesPropertyDelegate *delegate)
{
    nodesDelegate = delegate;
}

void NodesPropertyControl::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}
void NodesPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, 
                                               int32 newItemIndex, const String &newItemKey)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}

void NodesPropertyControl::OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4)
{
    if(nodesDelegate)
    {
        nodesDelegate->NodesPropertyChanged();
    }
}

void NodesPropertyControl::UpdateProjectPath()
{
    KeyedArchive *keyedArchieve = new KeyedArchive();
    keyedArchieve->Load("~doc:/ResourceEditorOptions.archive");
    projectPath = keyedArchieve->GetString("LastSavedPath", "/");
    if('/' != projectPath[projectPath.length() - 1])
    {
        projectPath += '/';
    }

    SafeRelease(keyedArchieve);
}

void NodesPropertyControl::OnPlus(BaseObject * object, void * userData, void * callerData)
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }

    AddControl(propControl);
}
void NodesPropertyControl::OnMinus(BaseObject * object, void * userData, void * callerData)
{
    if(propControl->GetParent() || listHolder->GetParent())
    {
        return;
    }
    
    KeyedArchive *customProperties = currentNode->GetCustomProperties();
    Map<String, VariantType> propsData = customProperties->GetArchieveData();

    float32 size = propsData.size();
    if(size)
    {
        Rect r = listHolder->GetRect();
        r.dy = Min(r.dy, size * CellHeight(NULL, 0));
        r.y = listHolder->GetRect().dy - r.dy - ControlsFactory::BUTTON_HEIGHT;
        
        deletionList = new UIList(r, UIList::ORIENTATION_VERTICAL);
//        ControlsFactory::CustomizeDialog(deletionList);
        ControlsFactory::CustomizePropertyCell(deletionList, false);

        deletionList->SetDelegate(this);
        
        listHolder->AddControl(deletionList);
        deletionList->Refresh();
        AddControl(listHolder);
    }
}

void NodesPropertyControl::NodeCreated(bool success)
{
    RemoveControl(propControl);
    if(success)
    {
        KeyedArchive *currentProperties = currentNode->GetCustomProperties();
        
        String name = propControl->GetPropName();
        switch (propControl->GetPropType()) 
        {
            case CreatePropertyControl::EPT_STRING:
                propertyList->AddStringProperty(name);
                propertyList->SetStringPropertyValue(name, "");
                
                currentProperties->SetString(name, "");
                break;

            case CreatePropertyControl::EPT_INT:
                propertyList->AddIntProperty(name);
                propertyList->SetIntPropertyValue(name, 0);
                
                currentProperties->SetInt32(name, 0);

                break;
            case CreatePropertyControl::EPT_FLOAT:
                propertyList->AddFloatProperty(name);
                propertyList->SetFloatPropertyValue(name, 0.f);
                
                currentProperties->SetFloat(name, 0.f);

                break;
            case CreatePropertyControl::EPT_BOOL:
                propertyList->AddBoolProperty(name);
                propertyList->SetBoolPropertyValue(name, false);
                
                currentProperties->SetBool(name, false);

                break;

            default:
                break;
        }
    }
}


int32 NodesPropertyControl::ElementsCount(UIList * list)
{
    KeyedArchive *customProperties = currentNode->GetCustomProperties();
    Map<String, VariantType> propsData = customProperties->GetArchieveData();
    
    return propsData.size();
}

UIListCell *NodesPropertyControl::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = (UIListCell *)list->GetReusableCell("Deletion list");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, 200, 20), "Deletion list");
    }
    
    KeyedArchive *customProperties = currentNode->GetCustomProperties();
    Map<String, VariantType> propsData = customProperties->GetArchieveData();
    int32 i = 0; 
    for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
    {
        if(i == index)
        {
            String name = it->first;

            ControlsFactory::CustomizeListCell(c, StringToWString(name));
            break;
        }
    }
    
    return c;
}

int32 NodesPropertyControl::CellHeight(UIList * list, int32 index)
{
    return CELL_HEIGHT;
}

void NodesPropertyControl::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    int32 index = selectedCell->GetIndex();
    KeyedArchive *customProperties = currentNode->GetCustomProperties();
    Map<String, VariantType> propsData = customProperties->GetArchieveData();
    int32 i = 0; 
    for (Map<String, VariantType>::iterator it = propsData.begin(); it != propsData.end(); ++it, ++i)
    {
        if(i == index)
        {
            WriteTo(currentNode);
            
            customProperties->DeleteKey(it->first);
            
            OnCancel(NULL, NULL, NULL);
            ReadFrom(currentNode);
            break;
        }
    }

}

void NodesPropertyControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    listHolder->RemoveControl(deletionList);
    SafeRelease(deletionList);
    RemoveControl(listHolder);
}

