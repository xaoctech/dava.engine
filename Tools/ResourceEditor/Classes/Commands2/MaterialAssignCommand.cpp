/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "MaterialAssignCommand.h"

#include "Scene/EntityGroup.h"


#include "DAVAEngine.h"

MaterialAssignCommand::MaterialAssignCommand()
	: Command2(CMDID_MATERIAL_ASSIGN, "Assign Material")
{
}

MaterialAssignCommand::~MaterialAssignCommand()
{
}

void MaterialAssignCommand::Undo()
{
}

void MaterialAssignCommand::Redo()
{
}

DAVA::Entity* MaterialAssignCommand::GetEntity() const
{
	return NULL;
}

bool MaterialAssignCommand::EntityGroupHasMaterials(EntityGroup *group, bool recursive)
{
    if(!group) return false;
    
    const size_t count = group->Size();
    for(size_t i = 0; i < count; ++i)
    {
        bool hasMaterials = EntityHasMaterials(group->GetEntity(i), recursive);
        if(hasMaterials) return true;
    }
    
    return false;
}

bool MaterialAssignCommand::EntityHasMaterials(DAVA::Entity * entity, bool recursive)
{
    if(!entity) return false;
    
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);
    if(ro && ro->GetRenderBatchCount())
    {
        return true;
    }
    
    if(recursive)
    {
        const DAVA::int32 count = entity->GetChildrenCount();
        for(DAVA::int32 i = 0; i < count; ++i)
        {
            bool hasMaterials = EntityHasMaterials(entity->GetChild(i), recursive);
            if(hasMaterials) return true;
        }
    }
    
    return false;
}
