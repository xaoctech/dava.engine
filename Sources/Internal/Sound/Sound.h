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

#ifndef __DAVAENGINE_SOUND_H__
#define __DAVAENGINE_SOUND_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/EventDispatcher.h"

namespace DAVA
{

class Sound : public BaseObject
{
public:
    enum eType
    {
        TYPE_STATIC = 0,
        TYPE_STREAMED
    };

    virtual void SetVolume(float32 volume) = 0;
    virtual float32	GetVolume() = 0;

    virtual void Play(const Message & msg = Message()) = 0;
    virtual void Pause(bool isPaused) = 0;
    virtual bool IsPaused() = 0;
    virtual void Stop() = 0;

    virtual void SetPosition(const Vector3 & position) = 0;
    virtual void UpdateInstancesPosition() = 0;

    virtual void SetLoopCount(int32 looping) = 0; // -1 = infinity
    virtual int32 GetLoopCount() = 0;

    eType GetType() {return type;};

protected:
    Sound(eType _type) : type(_type) {};
    virtual ~Sound() {};

    eType type;
};

};

#endif //__DAVAENGINE_SOUND_H__