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

#include "EditorHeightmap.h"

using namespace DAVA;

EditorHeightmap::EditorHeightmap(Heightmap *heightmap)
    :   Heightmap()
{
    DVASSERT(heightmap && "Can't be NULL");
    
    savedHeightmap = SafeRetain(heightmap);
    DownscaleOrClone();
}

EditorHeightmap::~EditorHeightmap()
{
    SafeRelease(savedHeightmap);
}


void EditorHeightmap::DownscaleOrClone()
{
    if(MAX_EDITOR_HEIGHTMAP_SIZE < savedHeightmap->Size())
    {
        Downscale(MAX_EDITOR_HEIGHTMAP_SIZE);
    }
    else
    {
        savedHeightmap->Clone(this);
    }
}
    
void EditorHeightmap::Downscale(int32 newSize)
{
    AllocateData(newSize);
    SetTileSize(savedHeightmap->GetTileSize());
    
    int32 savedHeightmapSize = savedHeightmap->Size() - 1;
    int32 heightmapSide = size - 1;
    int32 multiplier = savedHeightmapSize / heightmapSide;
    
    for(int32 y = 0; y < heightmapSide; ++y)
    {
        int32 yOffset = y * size;
        for(int32 x = 0; x < heightmapSide; ++x)
        {
            data[yOffset + x] = GetHeightValue(x, y, multiplier);
        }
    }
    
    for(int32 y = 0; y < heightmapSide; ++y)
    {
        data[y * size + (size - 1)] = GetVerticalValue(y, multiplier);
    }
    
    int32 yOffset = (size - 1) * size;
    for(int32 x = 0; x < heightmapSide; ++x)
    {
        data[yOffset + x] = GetHorizontalValue(x, multiplier);
    }
    
    data[size * size - 1] = savedHeightmap->Data()[savedHeightmap->Size() * savedHeightmap->Size() - 1]; //left bottom corner
}
    
    
bool EditorHeightmap::IsPowerOf2(int32 num)
{
    return ((num & (num - 1)) == 0);
}

uint16 EditorHeightmap::GetHeightValue(int32 posX, int32 posY, int32 muliplier)
{
    uint32 sum = 0;

    int32 firstX = posX * muliplier;
    int32 firstY = posY * muliplier;
    
    int32 lastX = firstX + muliplier;
    int32 lastY = firstY + muliplier;
    for(int32 y = firstY; y < lastY; ++y)
    {
        int32 yOffset = y * savedHeightmap->Size();
        for(int32 x = firstX; x < lastX; ++x)
        {
            sum += savedHeightmap->Data()[yOffset + x];
        }
    }

    return (uint16)(sum / (muliplier * muliplier));
}

uint16 EditorHeightmap::GetVerticalValue(int32 posY, int32 muliplier)
{
    uint32 sum = 0;
    
    int32 firstY = posY * muliplier;
    int32 lastY = firstY + muliplier;
    int32 index = firstY * savedHeightmap->Size() + savedHeightmap->Size() - 1;
    for(int32 y = firstY; y < lastY; ++y)
    {
        sum += savedHeightmap->Data()[index];
        index += savedHeightmap->Size();
    }
    
    return (uint16)(sum / muliplier);
}

uint16 EditorHeightmap::GetHorizontalValue(int32 posX, int32 muliplier)
{
    uint32 sum = 0;
    
    int32 firstX = posX * muliplier;
    int32 lastX = firstX + muliplier;
    int32 yOffset = (savedHeightmap->Size() - 1) * savedHeightmap->Size();
    for(int32 x = firstX; x < lastX; ++x)
    {
        sum += savedHeightmap->Data()[yOffset + x];
    }
    
    return (uint16)(sum / muliplier);
}

void EditorHeightmap::HeghtWasChanged(const DAVA::Rect &changedRect)
{
    int32 savedHeightmapSize = savedHeightmap->Size() - 1;
    int32 heightmapSide = size - 1;
    int32 multiplier = savedHeightmapSize / heightmapSide;

    int32 lastY = (int32)(changedRect.y + changedRect.dy);
    int32 lastX = (int32)(changedRect.x + changedRect.dx);
    for(int32 y = (int32)changedRect.y; y < lastY; ++y)
    {
        int32 yOffset = y * size;
        for(int32 x = (int32)changedRect.x; x < lastX; ++x)
        {
            SetHeightValue(x, y, multiplier, data[yOffset + x]);
        }
    }
}

void EditorHeightmap::SetHeightValue(DAVA::int32 posX, DAVA::int32 posY, DAVA::int32 muliplier, DAVA::uint16 value)
{
    int32 startX = posX * muliplier;
    int32 startY = posY * muliplier;
    
    int32 lastX = Min(startX + muliplier, savedHeightmap->Size() - 1);
    int32 lastY = Min(startY + muliplier, savedHeightmap->Size() - 1);
    
    for(int32 y = startY; y < lastY; ++y)
    {
        int32 yOffset = y * savedHeightmap->Size();
        for(int32 x = startX; x < lastX; ++x)
        {
            savedHeightmap->Data()[yOffset + x] = value;
        }
    }
}


void EditorHeightmap::Save(const DAVA::String &filePathname)
{
    savedHeightmap->Save(filePathname);
}

bool EditorHeightmap::Load(const DAVA::String &filePathname)
{
    bool loaded = savedHeightmap->Load(filePathname);
    DownscaleOrClone();
    
    return loaded;
}


