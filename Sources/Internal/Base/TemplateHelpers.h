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


#ifndef __DAVAENGINE_TEMPLATEHELPERS_H__
#define __DAVAENGINE_TEMPLATEHELPERS_H__

#include <typeinfo>

namespace DAVA
{

#define COMPILER_ASSERT(expr)  (DAVA::StaticAssert<(expr) != 0>());

// empty default template
template <bool b>
struct StaticAssert {};

// template specialized on true
template <>
struct StaticAssert<true>
{
	enum { Check = true };
};

template<bool C, typename T = void>
struct EnableIf
{
	typedef T type;
};

template<typename T>
struct EnableIf<false, T> 
{ };

template <int v>
struct Int2Type
{
    enum {value = v };
};
    
template <class T>
struct Type2Type
{
    typedef T OriginalType;
};
    
template <bool flag, typename T, typename U>
struct Select
{
    typedef T Result;
};
template <typename T, typename U>
struct Select<false, T, U>
{
    typedef U Result;
};

template <bool, unsigned int index_A, unsigned int index_B>
struct SelectIndex
{
	enum { result = index_A };
};

template <unsigned int index_A, unsigned int index_B>
struct SelectIndex<false, index_A, index_B>
{
	enum { result = index_B };
};
    
template <class TO, class FROM>
class Conversion
{
    typedef char Small;
    struct Big { char value[2]; };
    
    static Small Test(FROM);
    static Big Test(...);
    
    static TO MakeTO();
    
public:
    enum 
    {
        exists = 
        sizeof(Test(MakeTO())) == sizeof(Small)
    };
    
    enum 
    {
        sameType = false
    };
};

template<bool>
struct IsEnumImpl
{ };

template<>
struct IsEnumImpl<true>
{
	enum { result = true };
};

template<>
struct IsEnumImpl<false>
{
	enum { result = false };
};

template <class T>
struct IsEnum : public IsEnumImpl<__is_enum(T)>
{ };
    
template <class T>
class Conversion<T, T>
{
public:
    enum {exists = true };
    enum {sameType = true };
};
    
#define SUPERSUBCLASS(SUPER, SUB) (Conversion<const SUB*, const SUPER*>::exists && !Conversion<const SUPER*, const void*>::sameType) 
  

    /**
     \brief Works like dynamic_cast for Debug and like a static_cast for release.
     */
    template<class C, class O>
    C DynamicTypeCheck(O* pObject)
    {
#ifdef DAVA_DEBUG
        if(!pObject) return static_cast<C>(pObject);
        
        C c = dynamic_cast<C>(pObject);
        if (!c)
        {//assert emulation )
            int *i = NULL;
            *(i) = 0;
        }
        return c;
#else
        return static_cast<C>(pObject);
#endif
    }
    
    /**
     \brief Returns true if object pointer is a pointer to the exact class.
     */
    template<class C, class O>
    bool IsPointerToExactClass(const O* pObject) 
    {
		if (pObject)
        {
			COMPILER_ASSERT(!TypeTraits<C>::isPointer);//You should not use pointers for this method
			return &typeid(*pObject) == &typeid(C);
		}
	    return false;
    }
    
    template<class C, class O>
    C cast_if_equal(O* pObject)
    {
		if (pObject)
        {
			COMPILER_ASSERT(TypeTraits<C>::isPointer);
			if (typeid(*pObject) == typeid(typename PointerTraits<C>::PointerType))
            {
                return static_cast<C>(pObject);
            }
		}
	    return 0;
    }
    
    /* TEST, need to transfer to unit tests.
     Logger::FrameworkDebug("%d", Conversion<double, int>::exists);
     Logger::FrameworkDebug("%d", Conversion<Component*, VisibilityAABBoxComponent*>::exists);
     Logger::FrameworkDebug("%d", Conversion<VisibilityAABBoxComponent*, Component*>::exists);
     Logger::FrameworkDebug("%d", SUPERSUBCLASS(VisibilityAABBoxComponent, Component));
     Logger::FrameworkDebug("%d", SUPERSUBCLASS(Component, VisibilityAABBoxComponent));
     Logger::FrameworkDebug("%d", SUPERSUBCLASS(void*, VisibilityAABBoxComponent));
     Logger::FrameworkDebug("%d", SUPERSUBCLASS(BaseObject, VisibilityAABBoxComponent));
     
     
     Logger::FrameworkDebug("(BaseObject*) isPointer: %d", TypeTraits<BaseObject*>::isPointer);
     Logger::FrameworkDebug("(BaseObject*) isReference: %d", TypeTraits<BaseObject*>::isReference);
     Logger::FrameworkDebug("(BaseObject*) isPointerToMemberFunction: %d", TypeTraits<BaseObject*>::isPointerToMemberFunction);
     
     Logger::FrameworkDebug("(BaseObject&) isPointer: %d", TypeTraits<BaseObject&>::isPointer);
     Logger::FrameworkDebug("(BaseObject&) isReference: %d", TypeTraits<BaseObject&>::isReference);
     Logger::FrameworkDebug("(BaseObject&) isPointerToMemberFunction: %d", TypeTraits<BaseObject&>::isPointerToMemberFunction);
     
     void(VisibilityAABBoxSystem::*func)() = &VisibilityAABBoxSystem::Run;
     
     //    Logger::FrameworkDebug("(&VisibilityAABBoxSystem::Run) isPointer: %d", TypeTraits<VisibilityAABBoxSystem::Run>::isPointer);
     //    Logger::FrameworkDebug("(&VisibilityAABBoxSystem::Run) isReference: %d", TypeTraits<&VisibilityAABBoxSystem::Run>::isReference);
     //    Logger::FrameworkDebug("(&VisibilityAABBoxSystem::Run) isPointerToMemberFunction: %d", TypeTraits<&VisibilityAABBoxSystem::Run>::isPointerToMemberFunction);
     */    

    
    
};

#endif // __DAVAENGINE_TEMPLATEHELPERS_H__

