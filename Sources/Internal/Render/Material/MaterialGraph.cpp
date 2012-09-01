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
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"

namespace DAVA
{

    
// MaterialGraph
MaterialGraph::MaterialGraph()
{
    
}
    
MaterialGraph::~MaterialGraph()
{
    RemoveAllNodes();
}

void MaterialGraph::RemoveAllNodes()
{
    uint32 size = allNodes.size();
    for (uint32 k = 0; k < size; ++k)
    {
        SafeRelease(allNodes[k]);
    }
    allNodes.clear();
}
    
void MaterialGraph::AddNode(MaterialGraphNode * node)
{
    allNodes.push_back(SafeRetain(node));
}
    
void MaterialGraph::RemoveNode(MaterialGraphNode * node)
{
    allNodes.erase(std::remove(allNodes.begin(), allNodes.end(), node));
    SafeRelease(node);
}

bool MaterialGraph::LoadFromFile(const String & pathname)
{
    YamlParser * materialFileParser = YamlParser::Create(pathname);
    YamlNode * rootNode = materialFileParser->GetRootNode();
    
    YamlNode * nodes = rootNode->Get("nodes");
    if (nodes && nodes->GetType() == YamlNode::TYPE_ARRAY)
    {
        for (int32 k = 0; k < nodes->GetCount(); ++k)
        {
            YamlNode * graphNode = nodes->Get(k);
            bool result = LoadNode(graphNode);
            if (!result)break;
        }
    }
    
    SafeRelease(materialFileParser);
    return true;
}

bool MaterialGraph::LoadNode(YamlNode * graphNode)
{
    YamlNode * typeNode = graphNode->Get("type");
    YamlNode * nameNode = graphNode->Get("name");
    //YamlNode *
    
    
}
    
    
uint32 MaterialGraph::GetNodeCount() const
{
    return (uint32)allNodes.size();
}
MaterialGraphNode * MaterialGraph::GetNode(uint32 index) const
{
    return allNodes[index];
}
MaterialGraphNode * MaterialGraph::GetNodeByName(const String & name) const
{
    uint32 size = allNodes.size();
    for (uint32 k = 0; k < size; ++k)
    {
        if (allNodes[k]->GetName() == name)return allNodes[k];
    }
    return 0;
}

bool MaterialGraph::SortByDepthMarkerDescending(MaterialGraphNode * node1, MaterialGraphNode * node2)
{
    return (node1->GetDepthMarker() > node2->GetDepthMarker());
}

void MaterialGraph::SortByDepthMarker()
{
    std::sort(allNodes.begin(), allNodes.end(), SortByDepthMarkerDescending);
}


};

