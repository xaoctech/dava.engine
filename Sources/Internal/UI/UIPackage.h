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


#ifndef __DAVAENGINE_UI_PACKAGE_H__
#define __DAVAENGINE_UI_PACKAGE_H__

#include "Base/BaseObject.h"

namespace DAVA
{
class UIControl;
class UIStyleSheet;

class UIPackage: public BaseObject
{
public:
    UIPackage();

protected:
    ~UIPackage();
    
public:
    int32 GetControlsCount() const;
    UIControl *GetControl(int32 index) const;
    UIControl *GetControl(const String &name) const;
    
    template<class C>
    C GetControl(const String &name) const
    {
        return DynamicTypeCheck<C>(GetControl(name));
    }
    
    void AddControl(UIControl *control);
    void InsertControlAtIndex(DAVA::int32 index, UIControl *control);
    void InsertControlBelow(UIControl *control, const UIControl *belowThis);
    void RemoveControl(UIControl *control);


    RefPtr<UIPackage> Clone() const;
private:
    Vector<UIControl *> controls;

    UIControlPackageContext* controlPackageContext;
};

};
#endif // __DAVAENGINE_UI_PACKAGE_H__
