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

private:
	BASE_OBJECT * object;
	
	//noncopyable
	ScopedPtr(const ScopedPtr&);
	const ScopedPtr& operator=(const ScopedPtr&);
};

//implementation
template<typename BASE_OBJECT>
ScopedPtr<BASE_OBJECT>::ScopedPtr(BASE_OBJECT * p)
:	object(p)
{
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