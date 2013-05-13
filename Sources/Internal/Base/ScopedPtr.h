/*
 *  ScopedObject.h
 *  MahJongQuest
 *
 *  Created by Dizz on 11/27/09.
 *  Copyright 2009 1. All rights reserved.
 *
 */

#ifndef __DAVAENGINE_SCOPED_PTR_H__
#define __DAVAENGINE_SCOPED_PTR_H__

#include "Base/BaseTypes.h"
#include "FileSystem/Logger.h"

namespace DAVA
{

template<typename BASE_OBJECT>
class ScopedPtr
{
public:
	explicit ScopedPtr(BASE_OBJECT * p);
	~ScopedPtr();
	BASE_OBJECT & operator*() const;
	BASE_OBJECT * operator->() const;
	operator BASE_OBJECT*() const;
	
	//protection from 'delete ScopedObject'
	operator void*() const;
	
	ScopedPtr(const ScopedPtr&);
	const ScopedPtr& operator=(const ScopedPtr&);

private:
	BASE_OBJECT * object;
};

//implementation
template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::ScopedPtr(BASE_OBJECT * p)
:	object(p)
{
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::ScopedPtr(const ScopedPtr& scopedPtr)
{
	object = scopedPtr.object;
	object->Retain();
}

template<typename BASE_OBJECT>
const ScopedPtr<BASE_OBJECT>& ScopedPtr<BASE_OBJECT>::operator=(const ScopedPtr& scopedPtr)
{
	if (this == &scopedPtr)
	{
		return *this;
	}

	object = scopedPtr.object;
	object->Retain();

	return *this;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::~ScopedPtr()
{
	SafeRelease(object);
}

template<typename BASE_OBJECT>
BASE_OBJECT & ScopedPtr<BASE_OBJECT>::operator*() const
{
	return *object;
}

template<typename BASE_OBJECT>
BASE_OBJECT * ScopedPtr<BASE_OBJECT>::operator->() const
{
	return object;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::operator BASE_OBJECT*() const
{
	return object;
}

template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::operator void*() const
{
	return object;
}

};

#endif //__DAVAENGINE_SCOPED_PTR_H__