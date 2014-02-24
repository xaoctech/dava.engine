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

#ifndef __DAVAENGINE_GRASSRENDEROBJECT_H__
#define __DAVAENGINE_GRASSRENDEROBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
    
typedef Image GrassMap;
    
struct TextureSheetCell
{
    Vector2 t0;
    Vector2 t1;
    Vector2 t2;
    Vector2 t3;
};
    
class TextureSheet
{
    Vector<TextureSheetCell> cells;
    
    inline TextureSheet();
    inline ~TextureSheet();
    
    inline void SetTexture(Texture* tx);
    inline Texture* GetTexture() const;
    
private:
    
    Texture* texture;
};

class GrassRenderObject : public RenderObject
{
public:
        
    GrassRenderObject();
    virtual ~GrassRenderObject();
        
    RenderObject * Clone(RenderObject *newObject);
    virtual void Save(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Load(KeyedArchive *archive, SerializationContext *serializationContext);
    
    virtual void PrepareToRender(Camera *camera);

    void SetGrassMap(GrassMap* grassMap);
    const GrassMap* GetGrassMap() const;
    
    
    
private:
    
    void BuildGrassBrush(uint32 width, uint32 length);
    
private:
    
    GrassMap* grassMap;
};
    
inline TextureSheet::TextureSheet() : texture(NULL)
{
}
    
inline TextureSheet::~TextureSheet()
{
    SafeRelease(texture);
}
    
void TextureSheet::SetTexture(Texture* tx)
{
    if(tx != texture)
    {
        SafeRelease(texture);
        texture = SafeRetain(tx);
    }
}

Texture* TextureSheet::GetTexture() const
{
    return texture;
}

};

#endif
