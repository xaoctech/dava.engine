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
    
    tableOfChanges = NULL;
    InitializeTableOfChanges();
    
    scalingTable = NULL;
}

EditorHeightmap::~EditorHeightmap()
{
    SafeDeleteArray(scalingTable);
    
    SafeDeleteArray(tableOfChanges);
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

        data[y * size + (size - 1)] = GetVerticalValue(y, multiplier);
    }
    
    int32 yOffset = (size - 1) * size;
    for(int32 x = 0; x < heightmapSide; ++x)
    {
        data[yOffset + x] = GetHorizontalValue(x, multiplier);
    }
    
    data[size * size - 1] = savedHeightmap->Data()[savedHeightmap->Size() * savedHeightmap->Size() - 1]; //left bottom corner
}
    
void EditorHeightmap::InitializeTableOfChanges()
{
    SafeDeleteArray(tableOfChanges);
    tableOfChanges = new uint8[size * size];
    Memset(tableOfChanges, VALUE_NOT_CHANGED, size * size * sizeof(uint8));
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
    int32 lastY = (int32)(changedRect.y + changedRect.dy);
    int32 lastX = (int32)(changedRect.x + changedRect.dx);
    for(int32 y = (int32)changedRect.y; y < lastY; ++y)
    {
        int32 yOffset = y * size;
        for(int32 x = (int32)changedRect.x; x < lastX; ++x)
        {
            tableOfChanges[yOffset + x] = VALUE_WAS_CHANGED;
        }
    }
}


bool EditorHeightmap::Load(const DAVA::String &filePathname)
{
    bool loaded = savedHeightmap->Load(filePathname);
    DownscaleOrClone();
    
    InitializeTableOfChanges();
    
    return loaded;
}

void EditorHeightmap::Save(const DAVA::String &filePathname)
{
    if(savedHeightmap->Size() == size)
    {
        Memcpy(savedHeightmap->Data(), Data(), size * size * sizeof(uint16));
    }
    else
    {
        Upscale();
    }
    
    Memset(tableOfChanges, VALUE_NOT_CHANGED, size * size * sizeof(uint8));
    
    savedHeightmap->Save(filePathname);
}

void EditorHeightmap::Upscale()
{
    int32 savedHeightmapSize = savedHeightmap->Size() - 1;
    int32 heightmapSide = size - 1;
    int32 multiplier = savedHeightmapSize / heightmapSide;

    InitializeScalingTable(multiplier);
    
    for(int32 y = 0; y < size-1; ++y)
    {
        int32 yOffset = y * size;
        for(int32 x = 0; x < size-1; ++x)
        {
            int32 pos = yOffset + x;
            if(VALUE_WAS_CHANGED == tableOfChanges[pos])
            {
                UpscaleValue(x, y, multiplier);
            }
        }
        
        //last column
        if(VALUE_WAS_CHANGED == tableOfChanges[yOffset + size - 1])
        {
            uint16 top = data[yOffset + size - 1];
            uint16 bottom = data[yOffset + size + size - 1];
            int32 savedOffset = y * multiplier * savedHeightmap->Size() + savedHeightmap->Size() - 1;
            for(int32 i = 0; i < multiplier; ++i)
            {
                savedHeightmap->Data()[savedOffset + i * savedHeightmap->Size()] = (uint16)
                ((float32)top * (1.f - scalingTable[i]) + (float32)bottom * scalingTable[i]);
            }
        }
    }
    
    //LastRow
    int32 yOffset = (size-1) * size;
    int32 savedOffset = (savedHeightmap->Size() - 1) * savedHeightmap->Size();
    for(int32 x = 0; x < size-1; ++x)
    {
        if(VALUE_WAS_CHANGED == tableOfChanges[yOffset + x])
        {
            uint16 left = data[yOffset + x];
            uint16 right = data[yOffset + x + 1];
            
            int32 offset = savedOffset + x*multiplier;
            for(int32 i = 0; i < multiplier; ++i)
            {
                savedHeightmap->Data()[offset + i] = (uint16)
                ((float32)left * (1.f - scalingTable[i]) + (float32)right * scalingTable[i]);
            }
        }
    }
    
    if(VALUE_WAS_CHANGED == tableOfChanges[size * size - 1])
    {
        savedHeightmap->Data()[savedHeightmap->Size() * savedHeightmap->Size() - 1] = data[size * size - 1];
    }
}

void EditorHeightmap::InitializeScalingTable(DAVA::int32 count)
{
    DVASSERT((1 < count) && "Wrong count. Must be 2 or greater");
    
    SafeDeleteArray(scalingTable);
    scalingTable = new float32[count];
    
    float32 multiplier = 1.f / ((float32)count);
    for(int32 i = 0; i < count; ++i)
    {
        scalingTable[i] = (i + 0) * multiplier;
    }
}

void EditorHeightmap::UpscaleValue(DAVA::int32 leftX, DAVA::int32 topY, DAVA::int32 muliplier)
{
    int32 index = topY * size + leftX;
    uint16 topLeft = data[index];
    uint16 topRight = data[index + 1];
    uint16 bottomLeft = data[index + size];
    uint16 bottomRight = data[index + size + 1];
    
    
    int32 startX = leftX * muliplier;
    int32 startY = topY * muliplier;
    
    for(int32 y = 0; y < muliplier; ++y)
    {
        int32 offset = (startY + y) * savedHeightmap->Size() + startX;
        
        float32 v_ratio = scalingTable[y];
        float32 v_opposite = 1.f - v_ratio;

        for(int32 x = 0; x < muliplier; ++x)
        {
            float32 u_ratio = scalingTable[x];
            float32 u_opposite = 1.f - u_ratio;
            
            uint16 value = (uint16)(((float32)topLeft * u_opposite + (float32)topRight * u_ratio) * v_opposite +
                            ((float32)bottomLeft * u_opposite + (float32)bottomRight * u_ratio) * v_ratio);
            
            savedHeightmap->Data()[offset + x] = value;
        }
    }
}


