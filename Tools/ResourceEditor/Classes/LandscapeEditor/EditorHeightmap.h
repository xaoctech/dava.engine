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
=====================================================================================*/

#ifndef __EDITOR_HEIGHTMAP_H__
#define __EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"

class EditorHeightmap: public DAVA::Heightmap
{
    enum eConst
    {
        MAX_EDITOR_HEIGHTMAP_SIZE = 513
    };
    
public:

    EditorHeightmap(DAVA::Heightmap *heightmap);
	virtual ~EditorHeightmap();
    
    void HeghtWasChanged(const DAVA::Rect &changedRect);
    
    virtual void Save(const DAVA::String &filePathname);
    
protected:
    
    void Downscale(DAVA::int32 newSize);
    
    bool IsPowerOf2(DAVA::int32 num);

    DAVA::uint16 GetHeightValue(DAVA::int32 posX, DAVA::int32 posY, DAVA::int32 muliplier);
    DAVA::uint16 GetVerticalValue(DAVA::int32 posY, DAVA::int32 muliplier);
    DAVA::uint16 GetHorizontalValue(DAVA::int32 posX, DAVA::int32 muliplier);
    
    void SetHeightValue(DAVA::int32 posX, DAVA::int32 posY, DAVA::int32 muliplier, DAVA::uint16 value);
    
protected:

    Heightmap *savedHeightmap;
};


#endif //__EDITOR_HEIGHTMAP_H__
