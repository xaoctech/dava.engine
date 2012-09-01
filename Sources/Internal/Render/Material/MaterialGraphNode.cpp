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
#include "Render/Material/MaterialGraphNode.h"
#include "Utils/StringFormat.h"

namespace DAVA
{

/*
    Code for some of the nodes:

    Mul:
    var [name] = [a] * [b];
    
    Add:
    var [name] = [a] + [b];
    
    Lerp:
    var [name] = lerp([a], [b], [t]);
 
 
 
    <ambient diffuse>
    </>
    
    <diffuse> <diffuse>
 
 */
    
MaterialGraphNode::MaterialGraphNode(YamlNode * node)
    :   type(TYPE_NONE)
    ,   depthMarker(0)
    ,   isVertexShaderNode(false)
{
    
}
    
MaterialGraphNode::~MaterialGraphNode()
{
    for (Map<String, MaterialGraphNodeConnector*>::iterator it = inputConnectors.begin(); it != inputConnectors.end(); ++it)
    {
        SafeRelease(it->second);
    }
    inputConnectors.clear();
}

void MaterialGraphNode::SetDepthMarker(uint32 _depthMarker)
{
    depthMarker = _depthMarker;
}
    
uint32 MaterialGraphNode::GetDepthMarker()
{
    return depthMarker;
}

Map<String, MaterialGraphNodeConnector*> & MaterialGraphNode::GetInputConnectors()
{
    return inputConnectors;
}

    
void MaterialGraphNode::ConnectToNode(const String & connectorName, MaterialGraphNode * node, const String & connectionModifier)
{
    MaterialGraphNodeConnector * connector = new MaterialGraphNodeConnector();
    connector->node = node;
    connector->modifier = connectionModifier;
    inputConnectors[connectorName] = connector;
}

MaterialGraphNodeConnector * MaterialGraphNode::GetInputConnector(const String & name)
{
    Map<String, MaterialGraphNodeConnector*>::iterator res = inputConnectors.find(name);
    if (res != inputConnectors.end())
        return res->second;
    return 0;
}

MaterialGraphNode::eCompileError MaterialGraphNode::GenerateCode(String & vertexShader, String & pixelShader)
{
    // 
    if (type == TYPE_MUL)
    {
        MaterialGraphNodeConnector * connectorA = GetInputConnector("a");
        MaterialGraphNodeConnector * connectorB = GetInputConnector("b");
        
        if (!connectorA || !connectorB)
            return ERROR_NOT_ENOUGH_CONNECTORS;
        
        String shaderLine = Format("vec4 %s = %s.%s * %s.%s",  /*GetResultFormat(),*/
                                                            name.c_str(),
                                                            connectorA->node->GetName().c_str(),
                                                            connectorA->modifier.c_str(),
                                                            connectorB->node->GetName().c_str(),
                                                            connectorB->modifier.c_str());
        if (isVertexShaderNode)
            vertexShader += shaderLine;
        else
            pixelShader += shaderLine;
    
    }
    return NO_ERROR;
}

const String & MaterialGraphNode::GetName() const
{
    return name;
}
    
void MaterialGraphNode::SetName(const String & _name)
{
    name = _name;
}


};

