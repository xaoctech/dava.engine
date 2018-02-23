#pragma once
#include <Base/BaseTypes.h>

namespace DAVA
{
template <class T>
using Asset = std::shared_ptr<T>;
//redirecting_shared_ptr<reference_ptr<T>>;

template <class T>
using AssetWeakLink = std::weak_ptr<T>;
//std::weak_ptr<reference_ptr<T>>;

/*
 Lot of difference experiments to handle asset reload, leave them here for now.
 TODO: delete
 
template <class T>
class reference_ptr
{
public:
    reference_ptr(T* ptr_)
    {
        ptr = ptr_;
    }
    template<class U>
    reference_ptr(U* ptr_)
    {
        ptr = ptr_;
    }

    T* ptr = nullptr;
};
    
template<class T>
struct get_inner_type
{
    typedef T value_type;
};

template<template<typename> class X, typename T>
struct get_inner_type<X<T>>
{
    typedef T value_type;
};

template <class REF_PTR_T>
class redirecting_shared_ptr : public std::shared_ptr<REF_PTR_T>
{
public:
    using std::shared_ptr<REF_PTR_T>::shared_ptr;
    typedef typename get_inner_type<REF_PTR_T>::value_type ORIG;
    // T* operator->()
    // {
    //    return this->T::operator->();
    //}
    ORIG * operator->()
    {
        return std::shared_ptr<REF_PTR_T>::get()->ptr;
    }
};
*/

/*using AssetHandle = uint32;

template<class T>
class Asset
{
public:
    Asset(AssetHandle handle)
    {
        counter = 1;
    }
    ~Asset()
    {
        
    }
    
private:
    
    std::atomic<int32> counter;
};
*/
}
