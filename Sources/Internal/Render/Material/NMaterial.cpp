#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"


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

    
void NMaterialInstance::SetLightmap(Texture * texture, const String & lightmapName)
{
    
}
    
void NMaterialInstance::SetUVOffsetScale(const Vector2 & uvOffset, const Vector2 uvScale)
{
    
}

Texture * NMaterialInstance::GetLightmap() const
{
    return 0;
}
    
const String & NMaterialInstance::GetLightmapName() const
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
const FastName & NMaterialInstance::GetOwnerLayerName() const
{
    return FastName("");
}
    
void NMaterialInstance::SetOwnerLayerName(const FastName & _fastname)
{
    
}
    
NMaterialInstance * NMaterialInstance::Clone()
{
    NMaterialInstance * materialInstance = new NMaterialInstance();
    return materialInstance;
}

    
    
    
NMaterial::NMaterial()
{
}
    
NMaterial::~NMaterial()
{
}

bool NMaterial::LoadFromFile(const String & pathname)
{
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
    //YamlNode * pass =
    
    for (int32 k = 0; k < rootNode->GetCount(); ++k)
    {
        YamlNode * renderStepNode = rootNode->Get(k);
        
        if (renderStepNode->AsString() == "RenderStep")
        {
            Logger::Debug("- RenderStep found: %s", renderStepNode->AsString().c_str());
            YamlNode * shaderNode = renderStepNode->Get("Shader");
            YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");

            if (!shaderNode && !shaderGraphNode)
            {
                Logger::Error("Material:%s RenderStep:%s does not have shader or shader graph", pathname.c_str(), renderStepNode->AsString().c_str());
                SafeRelease(parser);
                return false;
            }
            
            String vertexShader, fragmentShader;
            if (shaderNode)
            {
                YamlNode * vertexShaderNode = shaderNode->Get(0);
                YamlNode * fragmentShaderNode = shaderNode->Get(1);
                
                DVASSERT(vertexShaderNode && fragmentShaderNode);
                
                vertexShader = vertexShaderNode->AsString();
                fragmentShader = fragmentShaderNode->AsString();
            }
            
            if (shaderGraphNode)
            {
                String shaderGraphPathname = shaderGraphNode->AsString();
                MaterialGraph * graph = new MaterialGraph();
                graph->LoadFromFile(shaderGraphPathname);

                MaterialCompiler * compiler = new MaterialCompiler();
                compiler->Compile(graph, 0, 4, 0);
                
                vertexShader = compiler->GetCompiledVertexShaderPathname();
                fragmentShader = compiler->GetCompiledFragmentShaderPathname();
                
                SafeRelease(compiler);
                SafeRelease(graph);
            }
            
            YamlNode * definesNode = renderStepNode->Get("Defines");
            if (definesNode)
            {
                
                
            }
            
            // YamlNode * n;
            
        } 
    }

    SafeRelease(parser);
    return true;
}
    
void NMaterial::AddShader(const FastName & fname, Shader * shader)
{
    DVASSERT(shader != 0);
    //shaders.Insert(fname, SafeRetain(shader));
}
    
uint32 NMaterial::GetShaderCount()
{
    return (uint32)shaders.Size();
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

void NMaterialInstance::Draw(PolygonGroup * polygonGroup)
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
