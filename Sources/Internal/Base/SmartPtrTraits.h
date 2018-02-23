#pragma once

#include <memory>
#include <type_traits>

namespace DAVA
{
template <typename T>
struct is_std_smart_ptr_impl : std::false_type
{
};
template <typename T>
struct is_std_smart_ptr_impl<std::shared_ptr<T>> : std::true_type
{
};
template <typename T>
struct is_std_smart_ptr_impl<std::unique_ptr<T>> : std::true_type
{
};
template <typename T>
struct is_std_smart_ptr_impl<std::weak_ptr<T>> : std::true_type
{
};
template <typename T>
struct is_std_smart_ptr : is_std_smart_ptr_impl<typename std::remove_cv<T>::type>
{
};

template <typename T>
struct remove_std_smart_ptr
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::shared_ptr<T>>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::shared_ptr<T> const>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::shared_ptr<T> volatile>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::shared_ptr<T> const volatile>
{
    typedef T type;
};

template <typename T>
struct remove_std_smart_ptr<std::unique_ptr<T>>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::unique_ptr<T> const>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::unique_ptr<T> volatile>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::unique_ptr<T> const volatile>
{
    typedef T type;
};

template <typename T>
struct remove_std_smart_ptr<std::weak_ptr<T>>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::weak_ptr<T> const>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::weak_ptr<T> volatile>
{
    typedef T type;
};
template <typename T>
struct remove_std_smart_ptr<std::weak_ptr<T> const volatile>
{
    typedef T type;
};

template <typename _Tp>
using remove_std_smart_ptr_t = typename remove_std_smart_ptr<_Tp>::type;
}
