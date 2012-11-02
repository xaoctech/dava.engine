#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderStateBlock.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"


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
    uniformShifts = 0;
    uniformData = 0;
    lightCount = 0;
    for (uint32 k = 0; k < 8; ++k)
    {
        lightNodes[k] = 0;
    }
    
    renderState.state = RenderStateBlock::DEFAULT_3D_STATE;
}
    
NMaterialInstance::~NMaterialInstance()
{
    SafeDeleteArray(uniformShifts);
    SafeDeleteArray(uniformData);
}
    
bool NMaterialInstance::LoadFromYaml(const String & pathname)
{
    YamlParser * parser = parser->Create(pathname);
    parser->GetRootNode();
    
    
    
    
    SafeRelease(parser);
}
    
void NMaterialInstance::SetUniformData(uint32 uniformIndex, void * data, uint32 size)
{
    memcpy(&uniformData[uniformShifts[uniformIndex]], data, size);
}

    
void NMaterialInstance::PrepareInstanceForShader(Shader * shader)
{
    uniformCount = shader->GetUniformCount();
    uniformShifts = new uint32[uniformCount];
    
    uint32 uniformDataSize = 0;
    for (int32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        uint32 size = shader->GetUniformTypeSize(uniformType) + 4;
        uniformShifts[uniformIndex] = uniformDataSize;
        uniformDataSize += size;
    }
    
    uniformData = new uint8[uniformDataSize];

    for (int32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        const String & uniformName = shader->GetUniformName(uniformIndex);
        
        if (uniformName == "texture[0]")
        {
            uint32 textureIndex = 0;
            SetUniformData(uniformIndex, &textureIndex, 4);
        }else if (uniformName == "texture[1]")
        {
            uint32 textureIndex = 1;
            SetUniformData(uniformIndex, &textureIndex, 4);
        }else if (uniformName == "texture[2]")
        {
            uint32 textureIndex = 2;
            SetUniformData(uniformIndex, &textureIndex, 4);
        }else if (uniformName == "texture[3]")
        {
            uint32 textureIndex = 3;
            SetUniformData(uniformIndex, &textureIndex, 4);
        }
    }
}
    
// TODO: Platform specific code.
// Either rethink, or rewrite. 

void NMaterialInstance::BindUniforms(Shader * shader)
{
    for (int32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
    {
        Shader::eUniformType uniformType = shader->GetUniformType(uniformIndex);
        uint8 * data = &uniformData[uniformShifts[uniformIndex]];
        switch(uniformType)
        {
            case Shader::UT_FLOAT:
                RENDER_VERIFY(glUniform1fv(shader->GetUniformLocation(uniformIndex), 1, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC2:
                RENDER_VERIFY(glUniform2fv(shader->GetUniformLocation(uniformIndex), 1, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC3:
                RENDER_VERIFY(glUniform3fv(shader->GetUniformLocation(uniformIndex), 1, (float*)data));
                break;
            case Shader::UT_FLOAT_VEC4:
                RENDER_VERIFY(glUniform4fv(shader->GetUniformLocation(uniformIndex), 1, (float*)data));
                break;
            case Shader::UT_INT:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_INT_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_INT_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_INT_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_BOOL:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC2:
                RENDER_VERIFY(glUniform2iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC3:
                RENDER_VERIFY(glUniform3iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_BOOL_VEC4:
                RENDER_VERIFY(glUniform4iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_FLOAT_MAT2:
                RENDER_VERIFY(glUniformMatrix2fv(shader->GetUniformLocation(uniformIndex), 1, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT3:
                RENDER_VERIFY(glUniformMatrix3fv(shader->GetUniformLocation(uniformIndex), 1, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_FLOAT_MAT4:
                RENDER_VERIFY(glUniformMatrix4fv(shader->GetUniformLocation(uniformIndex), 1, GL_FALSE, (float32*)data));
                break;
            case Shader::UT_SAMPLER_2D:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
            case Shader::UT_SAMPLER_CUBE:
                RENDER_VERIFY(glUniform1iv(shader->GetUniformLocation(uniformIndex), 1, (int32*)data));
                break;
        };
    }
}

    
NMaterial::NMaterial(uint32 _shaderCount)
    :   shaderCount(_shaderCount)
{
    shaderArray = new Shader*[shaderCount];
}
    
NMaterial::~NMaterial()
{
    SafeDeleteArray(shaderArray);
}
    
void NMaterial::SetShader(uint32 index, Shader * shader)
{
    DVASSERT(shaderArray != 0);
    shaderArray[index] = shader;
}
    
void NMaterial::PrepareRenderState(PolygonGroup * polygonGroup, NMaterialInstance * instance)
{
    
//    if (1)
    {
        shaderArray[0]->Dump();
    }
    
    
    Shader * shader = shaderArray[instance->GetLightCount()];
    DVASSERT(shader != 0)
    instance->renderState.SetShader(shader);
    RenderManager::Instance()->FlushState(&instance->renderState);
    RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
	RenderManager::Instance()->AttachRenderData();
};

void NMaterial::Draw(PolygonGroup * polygonGroup, NMaterialInstance * instance)
{
    

    
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
