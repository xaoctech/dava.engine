/*==================================================================================
    Copyright (c) 2013, DAVA Consulting, LLC
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
 * Created by Yury Danilov
   =====================================================================================*/
#pragma once
#include "DAVAEngine.h"
#include "physics2/p2conf.hpp"
#include "physics2/worldtri.hpp"
#include "physics2/bsp.hpp"

class ServerSPT
: public DAVA::BaseObject
{
public:
    ServerSPT(const DAVA::String& fileName, const DAVA::String& mapName, DAVA::Entity* model);

    const DAVA::String& GetModelName(void)
    {
        return modelName;
    };

    static const DAVA::Matrix4 globalTransform;
    inline bool IsWrong()
    {
        return isWrongModel;
    };
    void PrepareDestructible();
    void ReadCustomProperties(DAVA::Entity* model);

private:
    DAVA::String modelName;
    bool isWrongModel = false;
    DAVA::String mapName;
    DAVA::Entity* clone = nullptr;
    BSPTree* pBSP = nullptr;
    DAVA::int32 health = 5;
    DAVA::int32 fallType;
    DAVA::float32 density;
};
