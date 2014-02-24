//
//  GrassRenderObject.cpp
//  Framework
//
//  Created by Valentine Ivanov on 2/24/14.
//
//

#include "Render/Image.h"
#include "Render/Highlevel/GrassRenderObject.h"

namespace DAVA
{

GrassRenderObject::GrassRenderObject() :
    grassMap(NULL)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_CUSTOM_DRAW;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
}

GrassRenderObject::~GrassRenderObject()
{
    SafeRelease(grassMap);
}
    
RenderObject* GrassRenderObject::Clone(RenderObject *newObject)
{
    return NULL;
}

void GrassRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
    
void GrassRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
        
}
  
void GrassRenderObject::PrepareToRender(Camera *camera)
{
        
}
    
void GrassRenderObject::SetGrassMap(GrassMap* grassMap)
{
}
    
const GrassMap* GrassRenderObject::GetGrassMap() const
{
    return NULL;
}
    
void GrassRenderObject::BuildGrassBrush(uint32 width, uint32 length)
{
            
}


};