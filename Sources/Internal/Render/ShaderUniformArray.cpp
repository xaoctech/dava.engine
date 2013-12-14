#include "Render/Material/MaterialUniformArray.h"
#include "Render/RenderManager.h"
#include "Render/RenderStateBlock.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{

    
ShaderUniformArray::ShaderUniformArray(Shader * shader)
{
    uniformCount = shader->GetUniformCount();
    
}

ShaderUniformArray::~ShaderUniformArray()
{
    
}
    
        
void ShaderUniformArray::Set(uint32 uniformIndex, void * data, uint32 size)
{
    
}

};
