#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"


namespace DAVA
{
    
    
NMaterialDescriptor::NMaterialDescriptor()
{
    
}
    
NMaterialDescriptor::~NMaterialDescriptor()
{
    
}

uint32 NMaterialDescriptor::GetTextureSlotByName(const String & textureName)
{
    Map<String, uint32>::iterator it = slotNameMap.find(textureName);
    if (it != slotNameMap.end())
    {
        return it->second;
    }
    // if we haven't found slot let's use first slot
    return 0;
}
    
uint32 NMaterialDescriptor::GetUniformSlotByName(const String & uniformName)
{
    Map<String, uint32>::iterator it = uniformNameMap.find(uniformName);
    if (it != uniformNameMap.end())
    {
        return it->second;
    }
    // if we haven't found slot let's use first slot
    return 0;
}
    
void NMaterialDescriptor::SetNameForTextureSlot(uint32 slot, const String & name)
{
    slotNameMap[name] = slot;
}

void NMaterialDescriptor::SetNameForUniformSlot(uint32 slot, const String & name)
{
    uniformNameMap[name] = slot;
}


NMaterialInstance::NMaterialInstance()
{
    DVASSERT(sizeof(UniformInfo) == 4);
    uniforms = 0;
    uniformData = 0;
    lightCount = 0;
    for (uint32 k = 0; k < 8; ++k)
    {
        lights[k] = 0;
    }
    
    renderState.state = RenderState::DEFAULT_3D_STATE;
    shader = 0;
}
    
NMaterialInstance::~NMaterialInstance()
{
    SafeDeleteArray(uniforms);
    SafeDeleteArray(uniformData);
}
    
void NMaterialInstance::SetUniformData(uint32 uniformIndex, void * data, uint32 size)
{
    memcpy(&uniformData[uniforms[uniformIndex].shift], data, size);
}

    
void NMaterialInstance::PrepareInstanceForShader(Shader * _shader)
{
    if (shader == _shader)return;
    shader = _shader;
    /*
        TODO: rethink how to prepare instances for particular shaders of material.
     */
    
    SafeDeleteArray(uniforms);
    SafeDeleteArray(uniformData);
    
    uniformCount = shader->GetUniformCount();
    uniforms = new UniformInfo[uniformCount];
    memset(uniforms, 0, sizeof(UniformInfo) * uniformCount);
    
    uint32 uniformDataSize = 0;
    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        uint32 size = shader->GetUniformTypeSize(uniformType) * shader->GetUniformArraySize(uniformIndex) + 4;
        uniforms[uniformIndex].shift = uniformDataSize;
        uniforms[uniformIndex].arraySize = shader->GetUniformArraySize(uniformIndex);
        uniformDataSize += size;
    }
    
    uniformData = new uint8[uniformDataSize];

    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        //Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        const String & uniformName = shader->GetUniformName(uniformIndex);
        //Logger::Debug("Find uniform: %s", uniformName.c_str());
        
        if ((uniformName == "texture[0]") || (uniformName == "texture"))
        {
            uint32 textureIndexes[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            SetUniformData(uniformIndex, textureIndexes, 4 *  shader->GetUniformArraySize(uniformIndex));
        }else if ((uniformName == "lightIntensity[0]") || (uniformName == "lightIntensity"))
        {
            float32 lightIntensity[8] = {500, 500, 500, 500, 100, 100, 100, 100};
            SetUniformData(uniformIndex, lightIntensity, 4 *  shader->GetUniformArraySize(uniformIndex));
        }else if ((uniformName == "lightPosition[0]") || (uniformName == "lightPosition"))
        {
            Vector3 lightPosition[8] = {
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
                Vector3(0.0f, 0.0f, 0.0f),
            };
            SetUniformData(uniformIndex, lightPosition, 12 *  shader->GetUniformArraySize(uniformIndex));
        }else
        {
            uniforms[uniformIndex].flags |= SKIP_UNIFORM;
        }
    }
}
    
// TODO: Platform specific code.
// Either rethink, or rewrite. 

void NMaterialInstance::BindUniforms()
{
    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        UniformInfo uniformInfo = uniforms[uniformIndex];
        
        if (uniformInfo.flags & SKIP_UNIFORM)continue;
        
//        Logger::Debug("Bind uniform: %s", shader->GetUniformName(uniformIndex).c_str());
        
        uint8 * data = &uniformData[uniformInfo.shift];
        switch(uniformType)
        {
            case Shader::UT_FLOAT:
                RENDER_VERIFY(glUniform1fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC2:
                RENDER_VERIFY(glUniform2fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC3:
                RENDER_VERIFY(glUniform3fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC4:
                RENDER_VERIFY(glUniform4fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (float*)data));
                break;
            case Shader::UT_INT:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_INT_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_FLOAT_MAT2:
                RENDER_VERIFY(glUniformMatrix2fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT3:
                RENDER_VERIFY(glUniformMatrix3fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT4:
                RENDER_VERIFY(glUniformMatrix4fv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_SAMPLER_2D:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
            case Shader::UT_SAMPLER_CUBE:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), uniformInfo.arraySize, (int32*)data));
                break;
        };
    }
}

    
void NMaterialInstance::UpdateUniforms()
{
    // Get global time
    //float32 time = 0.0;
    //shader->GetUniformLocationByName(FN_GLOBAL_TIME);
    
    
    /*
         Camera * camera = scene->GetCurrentCamera();
         LightNode * lightNode0 = instanceMaterialState->GetLight(0);
         if (lightNode0 && camera)
         {
         if (uniformLightPosition0 != -1)
         {
         const Matrix4 & matrix = camera->GetMatrix();
         Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
     */
}
    
void NMaterialInstance::SetTwoSided(bool isTwoSided)
{
    if (isTwoSided)
        renderState.RemoveState(RenderState::STATE_CULL);
    else
        renderState.AppendState(RenderState::STATE_CULL);
}
    
bool NMaterialInstance::IsTwoSided()
{
    return (renderState.GetState() & RenderState::STATE_CULL) == 0;
}

    
void NMaterialInstance::SetLightmap(Texture * texture, const FilePath & lightmapName)
{
    
}
    
void NMaterialInstance::SetUVOffsetScale(const Vector2 & uvOffset, const Vector2 uvScale)
{
    
}

Texture * NMaterialInstance::GetLightmap() const
{
    return 0;
}
    
String NMaterialInstance::GetLightmapName() const
{
    return String();
}
    
void NMaterialInstance::Save(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
    
}
void NMaterialInstance::Load(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
    
}
    
bool NMaterialInstance::IsExportOwnerLayerEnabled() const
{
    return true;
}
void NMaterialInstance::SetExportOwnerLayer(const bool & isEnabled)
{
    
}
FastName NMaterialInstance::GetOwnerLayerName() const
{
    return FastName("");
}
    
void NMaterialInstance::SetOwnerLayerName(const FastName & _fastname)
{
    
}
    

void NMaterialInstance::PrepareRenderState()
{
    
    //    if (1)
    //    {
    //        shaderArray[0]->Dump();
    //    }
    
    
    renderState.SetShader(shader);
    PrepareInstanceForShader(shader);
    
    // Bind shader & global uniforms
    // TODO: Rethink RenderManager uniforms...
    RenderManager::Instance()->FlushState(&renderState);
    
    // Here bind local uniforms of material
    UpdateUniforms();
    BindUniforms();
    
};
    
NMaterialInstance * NMaterialInstance::Clone()
{
    NMaterialInstance * materialInstance = new NMaterialInstance();
    return materialInstance;
}

    
    
MaterialTechnique::MaterialTechnique(const FastName & _shaderName, FastNameSet & _uniqueDefines, RenderState * _renderState)
{
    shader = 0;
    shaderName = _shaderName;
    uniqueDefines = _uniqueDefines;
    renderState = _renderState;
}
    
MaterialTechnique::~MaterialTechnique()
{
    SafeRelease(shader);
}

void MaterialTechnique::RecompileShader()
{
    SafeRelease(shader);
    shader = ShaderCache::Instance()->Get(shaderName, uniqueDefines);
}
    
NMaterial::NMaterial()
    : parent(0)
{
    activeTechnique = 0;
}
    
NMaterial::~NMaterial()
{
}
    
void NMaterial::AddMaterialProperty(const String & keyName, YamlNode * uniformNode)
{
    FastName uniformName = keyName;
    Logger::Debug("Uniform Add:%s %s", keyName.c_str(), uniformName.c_str());
    
    Shader::eUniformType type = Shader::UT_FLOAT;
    union 
    {
        float val;
        float valArray[4];
        float valMatrix[4 * 4];
        int32 valInt;
    }data;
    
    uint32 size = 0;
    
    if (uniformNode->GetType() == YamlNode::TYPE_STRING)
    {
        String uniformValue = uniformNode->AsString();
        if (uniformValue.find('.') != String::npos)
            type = Shader::UT_FLOAT;
        else
            type = Shader::UT_INT;
    }
    else if (uniformNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        size = uniformNode->GetCount();
        uint32 arrayCount = 0;
        for (uint32 k = 0; k < size; ++k)
        {
            if (uniformNode->Get(k)->GetType() == YamlNode::TYPE_ARRAY)
            {
                arrayCount++;
            }
        }
        if (size == arrayCount)
        {
            if (size == 2)type = Shader::UT_FLOAT_MAT2;
            else if (size == 3)type = Shader::UT_FLOAT_MAT3;
            else if (size == 4)type = Shader::UT_FLOAT_MAT4;
        }else if (arrayCount == 0)
        {
            if (size == 2)type = Shader::UT_FLOAT_VEC2;
            else if (size == 3)type = Shader::UT_FLOAT_VEC3;
            else if (size == 4)type = Shader::UT_FLOAT_VEC4;
        }else
        {
            DVASSERT(0 && "Something went wrong");
        }
    }
    
 
    
    switch (type) {
        case Shader::UT_INT:
            data.valInt = uniformNode->AsInt();
            break;
        case Shader::UT_FLOAT:
            data.val = uniformNode->AsFloat();
            break;
        case Shader::UT_FLOAT_VEC2:
        case Shader::UT_FLOAT_VEC3:
        case Shader::UT_FLOAT_VEC4:
            for (uint32 k = 0; k < size; ++k)
            {
                data.valArray[k] = uniformNode->Get(k)->AsFloat();
            }
            break;
            
        default:
            Logger::Error("Wrong material property or format not supported.");
            break;
    }
    
    NMaterialProperty * materialProperty = new NMaterialProperty();
    materialProperty->size = 1;
    materialProperty->type = type;
    materialProperty->data = new char[Shader::GetUniformTypeSize(type)];
    
    memcpy(materialProperty->data, &data, Shader::GetUniformTypeSize(type));
    
    int32 t = *(int32*)materialProperty->data;
    
    materialProperties.Insert(uniformName, materialProperty);
}
    
void NMaterial::SetPropertyValue(const FastName & propertyFastName, Shader::eUniformType type, uint32 size, const void * data)
{
    NMaterialProperty * materialProperty = materialProperties.GetValue(propertyFastName);
    if (materialProperty)
    {
        if (materialProperty->type != type || materialProperty->size != size)
        {
            SafeDelete(materialProperty->data);
            materialProperty->size = size;
            materialProperty->type = type;
            materialProperty->data = new char[Shader::GetUniformTypeSize(type) * size];
        }
    }else
    {
        materialProperty = new NMaterialProperty;
        materialProperty->size = size;
        materialProperty->type = type;
        materialProperty->data = new char[Shader::GetUniformTypeSize(type) * size];
        materialProperties.Insert(propertyFastName, materialProperty);
    }

    memcpy(materialProperty->data, data, size * Shader::GetUniformTypeSize(type));
}


bool NMaterial::LoadFromFile(const String & pathname)
{
    materialName = pathname;
    YamlParser * parser = YamlParser::Create(pathname);
    if (!parser)
    {
        Logger::Error("Can't load requested material: %s", pathname.c_str());
        return false;
    }
    YamlNode * rootNode = parser->GetRootNode();
    
    if (!rootNode)
    {
        SafeRelease(rootNode);
        return false;
    }
    
    YamlNode * materialNode = rootNode->Get("Material");

    YamlNode * layersNode = materialNode->Get("Layers");
    if (layersNode)
    {
        int32 count = layersNode->GetCount();
        for (int32 k = 0; k < count; ++k)
        {
            YamlNode * singleLayerNode = layersNode->Get(k);
            layers.Insert(FastName(singleLayerNode->AsString()));
        }
    }
    
    YamlNode * uniformsNode = materialNode->Get("Uniforms");
    if (uniformsNode)
    {
        uint32 count = uniformsNode->GetCount();
        for (uint32 k = 0; k < count; ++k)
        {
            YamlNode * uniformNode = uniformsNode->Get(k);
            if (uniformNode)
            {
                AddMaterialProperty(uniformsNode->GetItemKeyName(k), uniformNode);
            }
        }
    }

    for (int32 k = 0; k < rootNode->GetCount(); ++k)
    {
        YamlNode * renderStepNode = rootNode->Get(k);
        
        if (renderStepNode->AsString() == "RenderPass")
        {
            Logger::Debug("- RenderPass found: %s", renderStepNode->AsString().c_str());
            YamlNode * shaderNode = renderStepNode->Get("Shader");
            YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");

            if (!shaderNode && !shaderGraphNode)
            {
                Logger::Error("Material:%s RenderPass:%s does not have shader or shader graph", pathname.c_str(), renderStepNode->AsString().c_str());
                SafeRelease(parser);
                return false;
            }
            
            FastName shaderName;
            if (shaderNode)
            {
                shaderName = FastName(shaderNode->AsString().c_str());
            }
            
            if (shaderGraphNode)
            {
                String shaderGraphPathname = shaderGraphNode->AsString();
                MaterialGraph * graph = new MaterialGraph();
                graph->LoadFromFile(shaderGraphPathname);

                MaterialCompiler * compiler = new MaterialCompiler();
                compiler->Compile(graph, 0, 4, 0);
                
                //vertexShader = compiler->GetCompiledVertexShaderPathname();
                //fragmentShader = compiler->GetCompiledFragmentShaderPathname();
                
                SafeRelease(compiler);
                SafeRelease(graph);
            }
            
            FastNameSet definesSet;
            YamlNode * definesNode = renderStepNode->Get("UniqueDefines");
            if (definesNode)
            {
                int32 count = definesNode->GetCount();
                for (int32 k = 0; k < count; ++k)
                {
                    YamlNode * singleDefineNode = definesNode->Get(k);
                    definesSet.Insert(FastName(singleDefineNode->AsString().c_str()));
                }
            }
            
            RenderState * renderState = new RenderState();
            YamlNode * renderStateNode = renderStepNode->Get("RenderState");
            if (renderStepNode)
            {
                renderState->LoadFromYamlNode(renderStepNode);
            }
            
            
            YamlNode * renderPassNameNode = renderStepNode->Get("Name");
            FastName renderPassName;
            if (renderPassNameNode)
            {
                renderPassName = renderPassNameNode->AsString();
            }
                        
            MaterialTechnique * technique = new MaterialTechnique(shaderName, definesSet, renderState);
            AddMaterialTechnique(renderPassName, technique);
            technique->RecompileShader();
        }
    }

    SafeRelease(parser);
    return true;
}
    
void NMaterial::AddMaterialTechnique(FastName & techniqueName, MaterialTechnique * materialTechnique)
{
    techniqueForRenderPass.Insert(techniqueName, materialTechnique);
}
    
MaterialTechnique * NMaterial::GetTechnique(const FastName & techniqueName)
{
    MaterialTechnique * technique = techniqueForRenderPass.GetValue(techniqueName);
    DVASSERT(technique != 0);
    return technique;
}
    
    
void NMaterial::SetTexture(const FastName & textureFastName, Texture * texture)
{
    textures.Insert(textureFastName, texture);
}
    
Texture * NMaterial::GetTexture(const FastName & textureFastName) const
{
    return textures.GetValue(textureFastName);
}


    
void NMaterial::BindMaterialTechnique(const FastName & techniqueName)
{
    if (techniqueName != activeTechniqueName)
    {
        activeTechnique = GetTechnique(techniqueName);
        if (activeTechnique)
            activeTechniqueName = techniqueName;
    }
    
    Shader * shader = activeTechnique->GetShader();
    activeTechnique->GetRenderState()->SetShader(shader);

    RenderManager::Instance()->FlushState(activeTechnique->GetRenderState());

    uint32 uniformCount = shader->GetUniformCount();

    for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
        if (uniform->id == Shader::UNIFORM_NONE)
        {
            NMaterial * currentMaterial = this;
            
            while(currentMaterial != 0)
            {
                NMaterialProperty * property = materialProperties.GetValue(uniform->name);
                if (property)
                {
                    Vector2 * dataVec2 = (Vector2 *)property->data;
                    int32 * dataInt32 = (int32 *)property->data;
                    
                    
                    shader->SetUniformValue(uniform->location, uniform->type, uniform->size, property->data);
                    break;
                }
                currentMaterial = currentMaterial->parent;
            }
        }
    }
}

//uint32 NMaterial::GetShaderCount()
//{
//    return (uint32)shaders.Size();
//}



void NMaterial::Draw(PolygonGroup * polygonGroup)
{
    // TODO: Remove support of OpenGL ES 1.0 from attach render data
    RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
	RenderManager::Instance()->AttachRenderData();

    // TODO: rethink this code
    if (polygonGroup->renderDataObject->GetIndexBufferID() != 0)
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
    }
    else
    {
        RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
    }
};
    



};
