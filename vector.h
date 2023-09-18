#ifndef MYTINYSTL_VECTOR_H_
#define MYTINYSTL_VECTOR_H_

// 这个头文件包含一个模板类 vector
// vector : 向量

// notes:
//
// 异常保证：
// mystl::vecotr<T> 满足基本异常保证，部分函数无异常保证，并对以下函数做强异常安全保证：
//   * emplace
//   * emplace_back
//   * push_back
// 当 std::is_nothrow_move_assignable<T>::value == true 时，以下函数也满足强异常保证：
//   * reserve
//   * resize
//   * insert

#include <initializer_list>

#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"
#include "algo.h"

namespace mystl
{

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

// 模板类: vector 
// 模板参数 T 代表类型
template<class T>
class vector
{
    static_assert(!std::is_same<bool, T>::value, "vector<bool> is abandoned in mystl");
public:
    // vector 的嵌套型别定义
    typedef mystl::allocator<T>                      allocator_type;
    typedef mystl::allocator<T>                      data_allocator;

    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference cost_reference;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;

    typedef value_type* iterator;
    typedef value_type*                              iterator;
    typedef const value_type*                        const_iterator;
    typedef mystl::reverse_iterator<iterator>        reverse_iterator;
    typedef mystl::reverse_iterator<const_iterator>  const_reverse_iterator;
    
    allocator_type get_allocator() { return data_allocator(); }
    /*
    **这段代码是在定义一系列类型别名，用于简化对allocator_type成员类型的访问。
    */

private:
    iterator begin_; // 表示目前使用空间的头部
    iterator end_;   // 表示目前使用空间的尾部
    iterator cap_;   // 表示目前储存空间的尾部

public:
    //构造、复制、移动、析构函数
    vector() noexcept
    { try_init(); }

    explicit vector(size_type n)
    { fill_init(n,value_type()); }

    vector(size_type n, const value_type& value)
    { fill_init(n, value); }

    template <class Iter, typename std::enable_if<
     mystl::is_input_iterator<Iter>::value,int>::type = 0>
    vector(Iter first, Iter last)
    {
        MYSTL_DEBUG(!(last<first));
        range_init(first,last);
    } 

    vector(const vector& rhs)
    {
        range_init(rhs.begin_, rhs.end_);
    }

    vector(vector&& rhs) noexcept
        :begin_(rhs.begin_),
        end_(rhs.end_),
        cap_(rhs.cap_)
    {
        rhs.begin_ = nullptr;
        rhs.end_ = nullptr;
        rhs.cap_ = nullptr;
    }

    vector(std::initializer_list<value_type> ilist)
    {
        range_init(ilist.begin(),ilist.end());
    }

    vector& operator=(const vector& rhs);
    vector& operator=(vector&& rhs);

    vector& operator=(std::initializer_list<value_type> ilist)
    {
        vector tmp(ilist.begin(), ilist.end());
        swap(tmp);
        return *this;
    }

    ~vector()
    {
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = end_ = cap_ =nullptr;    
    };

    public:

        // 迭代器相关操作
        iterator begin() noexcept
        { return begin_; }
        const_iterator begin() const noexcept
        { return begin_; }
        iterator end() noexcept
        { return end_; }
        const_iterator end() const noexcept
        { return end_; }

        reverse_iterator rbegin() noexcept
        { retrun reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept
        { return reverse_iterator(end()); }
        reverse_iterator rend() noexcept
        { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept
        { return reverse_iterator(begin()); }

        const_iterator cbegin() const noexcept
        { return begin(); }
        const_iterator cend() const noexcept
        { return cend(); }
        const_reverse_iterator crbegin() const noexcept
        { return rbegin(); }
        const_reverse_iterator crend() cosnt noexcept
        { return rend(); }

        // 容量相关操作
        bool empty() const noexcept
        { return begin_ == end_; }
        size_type size() cosnt noexcept
        { return static_cast<size_type>(end_- begin_); }
        size_type max_size() const noexcept
        { return static_cast<size_type>(-1) / sizeof(T); }
        size_type capacity() const noexcept
        { return static_cast<size_type>(cap_ - begin_); }
        void reverse(size_type n);
        void shrink_to_fit();

        // 访问元素相关操作
        reference operator[](size_type n)
        {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }
        cost_reference operator[](size_type n)
        {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }
        reference at(size_type n)
        {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
            return (*this)[n];  
        }
        const_reference at(size_type n)
        {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range");
            return (*this)[n];
        }

        reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }
        cost_reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }
          reference back()
        {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }
        const_reference back() const
        {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }

        pointer data() noexcept { return begin_; }
        const_pointer data() const noexcept {retunr begin_; }

        // 修改容器相关操作

        // assign

        void assgin(size_type n, const value_type& value)
        { fill_assgin(n, value); }

        template <class Iter, typename std::enable_if<
        mystl::is_input_iterator<Iter>::value, int>::type = 0>
        void assign(Iter first, Iter last)
        { 
            MYSTL_DEBUG(!(last < first));
            copy_assign(first, last, iterator_category(first));
        }

        void assign(std::initializer_list<value_type> il)
        { copy_assign(il.begin(), il.end(), mystl::forward_iterator_tag{}); }

    private:
        // helper fuctions

        // initialize / destroy
        void try_init() noexcept;
        
        void init_space(size_type size, size_type cap);
};
/*************************************************************************/
//  helper functions

// try_init 函数，若分配失败则忽略，不抛出异常
template <class T>
void vector<T>::try_init() noexcept
{
    try{
        begin_ = data_allocator::allocate(16);
        end_   = begin_;
        cap_   = begin_ + 16;
    }
    catch (...)
    {
        begin_ = nullptr;
        end_   = nullptr;
        cap_   = nullptr;
    }
}

// init_space 函数
template <class T>
void vector<T>::init_space(size_type size, size_type cap)
{
    try
    {   
        begin_ = data_allocate(cap);
        end_ = begin_ + size;
        cap_ = begin_ + cap; 
    }
    catch(...)
    {
        begin_ = nullptr;
        end_ = nullptr;
        cap_ = nullptr;
        throw;
    }
} 

}//namespace mystl end
#endif // !MYTINYSTL_VECTOR_H_