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
#ifndef __DAVAENGINE_MATERIAL_GRAPH_NODE_H__
#define __DAVAENGINE_MATERIAL_GRAPH_NODE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{
class MaterialGraphNode;
class MaterialGraphNodeConnector : public BaseObject
{
public:
    MaterialGraphNode * node;
    String modifier;
};

class MaterialGraphNode : public BaseObject
{
public:
    enum eCompileError
    {
        NO_ERROR = 1,
        ERROR_NOT_ENOUGH_CONNECTORS,
        ERROR_UNUSED_NODE,
    };
    
    enum eType
    {
        TYPE_NONE = 0,
        TYPE_FORWARD_MATERIAL,
        TYPE_DEFERRED_MATERIAL,
        TYPE_SAMPLE_2D,
        TYPE_MUL,
        TYPE_ADD,
        TYPE_LERP,
        TYPE_TIME,
        TYPE_SIN,
        TYPE_COS,
        
        TYPE_COUNT,
    };
    
    MaterialGraphNode();
    ~MaterialGraphNode();
    
    void SetDepthMarker(uint32 depthMarker);
    uint32 GetDepthMarker();
    
    MaterialGraphNodeConnector * GetInputConnector(const String & name);
    Map<String, MaterialGraphNodeConnector*> & GetInputConnectors();
    void ConnectToNode(const String & inputName, MaterialGraphNode * node, const String & connectionModifier);

    eCompileError GenerateCode(String & vertexShader, String & pixelShader);
    
    uint32 GetOutputCount();
    uint32 GetConstCount() const;
    
    const String & GetName() const;
    void SetName(const String & name);
    void SetType(const String & type);
    
    void MergeConnectionModifiers(const String & usedByOtherNode);
    
protected:
    String GetResultFormat(const String & s1, const String & s2);

    eType type;
    uint32 depthMarker;
    String name;
    bool isVertexShaderNode;
    String usedByOthersModifier;
    Map<String, MaterialGraphNodeConnector*> inputConnectors;
};

};

#endif // __DAVAENGINE_MATERIAL_GRAPH_NODE_H__

