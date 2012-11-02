/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/NMaterial.h"
#include "FileSystem/FileSystem.h"
#include "Render/Shader.h"

namespace DAVA
{
    
MaterialCompiler::eCompileResult MaterialCompiler::Compile(MaterialGraph * materialGraph, uint32 maxLights, NMaterial ** resultMaterial)
{
    MaterialGraphNode * rootResultNode = materialGraph->GetNodeByName("material");
    if (!rootResultNode)
    {
        return COMPILATION_FAILED;
    }

    RecursiveSetDepthMarker(rootResultNode, 0);
    materialGraph->SortByDepthMarker();
    GenerateCode(materialGraph);
    
    currentMaterial = new NMaterial(maxLights);

    
    
    Shader * shader = new Shader();
    shader->Load("~doc:/temp.vsh", "~doc:/temp.fsh");
    shader->Recompile();
    
    currentMaterial->SetShader(0, shader);
    
    *resultMaterial = currentMaterial;
    
    return COMPILATION_SUCCESS;
};
    
void MaterialCompiler::RecursiveSetDepthMarker(MaterialGraphNode * node, uint32 depthMarker)
{
    node->SetDepthMarker(depthMarker);
    
    Map<String, MaterialGraphNodeConnector*> & inputConnectors = node->GetInputConnectors();
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        MaterialGraphNodeConnector * connector = it->second;
        MaterialGraphNode * connectedNode = connector->node;
        if (connectedNode)
        {
            RecursiveSetDepthMarker(connectedNode, depthMarker + 1);
        }
    }
}

void MaterialCompiler::GenerateCode(MaterialGraph * materialGraph)
{
    // Generate pixel shader code
    String pixelShader;
    String vertexShader;
    
    for (uint32 nodeIndex = 0; nodeIndex < materialGraph->GetNodeCount(); ++nodeIndex)
    {
        MaterialGraphNode * node = materialGraph->GetNode(nodeIndex);
        node->GenerateCode(vertexShader, pixelShader);
    }
    
    
    MaterialGraphNode * rootResultNode = materialGraph->GetNodeByName("material");

    String vertexShaderPath = "~res:/Materials/forward.vsh";
    String fragmentShaderPath = "~res:/Materials/forward.fsh";

    String originalVertexShader = FileSystem::Instance()->ReadFileContents(vertexShaderPath);
    String originalFragmentShader = FileSystem::Instance()->ReadFileContents(fragmentShaderPath);
    
    
    File * resultVsh = File::Create("~doc:/temp.vsh", File::CREATE | File::WRITE);
    resultVsh->WriteString(rootResultNode->GetFragmentShaderCode(), false); // Save without null terminator
    resultVsh->WriteString(originalVertexShader);
    SafeRelease(resultVsh);
    
    File * resultFsh = File::Create("~doc:/temp.fsh", File::CREATE | File::WRITE);
    resultFsh->WriteString(rootResultNode->GetFragmentShaderCode(), false); // Save without null terminator
    resultFsh->WriteString(originalFragmentShader);
    //resultFsh->Write(fragmentShaderData->GetPtr(), fragmentShaderData->GetSize());
    SafeRelease(resultFsh);
};

};

