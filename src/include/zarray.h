
/*
* zarray License
* Copyright (C) 2019 YaweiZhang <yawei.zhang@foxmail.com>.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/





#ifndef  ZARRAY_H
#define ZARRAY_H

#include <type_traits>

namespace zsummer
{
    using s8 = char;
    using u8 = unsigned char;
    using s16 = short int;
    using u16 = unsigned short int;
    using s32 = int;
    using u32 = unsigned int;
    using s64 = long long;
    using u64 = unsigned long long;
    using f32 = float;
    using f64 = double;

#if __GNUG__
#define MAY_ALIAS __attribute__((__may_alias__))
#else
#define MAY_ALIAS
#endif

    template<class _Ty, u32 _Size>
    class zarray
    {
    public:
        using value_type = _Ty;
        using size_type = u32;
        constexpr static size_type size_value = _Size;
        using difference_type = ptrdiff_t;
        using pointer = _Ty*;
        using const_pointer = const _Ty*;
        using reference = _Ty&;
        using const_reference = const _Ty&;

        using iterator = pointer;
        using const_iterator = const_pointer;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using inner_space_type = typename std::aligned_storage<sizeof(_Ty), alignof(_Ty)>::type;
        using space_type = typename std::conditional<std::is_trivial<_Ty>::value, _Ty, inner_space_type>::type;
    private:
        pointer MAY_ALIAS ptr(size_type i) const noexcept { return reinterpret_cast<pointer>(const_cast<space_type*>(&data_[i])); }
        reference ref(size_type i) const noexcept { return *ptr(i); }
        size_type distance(const_pointer l, const_pointer r) const noexcept { return (size_type)(r - l); }
    public:
        zarray() { count_ = 0; }
        ~zarray() { clear(); }
        zarray(std::initializer_list<_Ty> init_list)
        {
            count_ = 0;
            assign(init_list.begin(), init_list.end());
        }

        //std::array api
        iterator begin() noexcept { return ptr(0); }
        const_iterator begin() const noexcept { return ptr(0); }
        const_iterator cbegin() const noexcept { return ptr(0); }

        iterator end() noexcept { return ptr(0) + count_; }
        const_iterator end() const noexcept { return ptr(0) + count_; }
        const_iterator cend() const noexcept { return ptr(0) + count_; }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
        const_reverse_iterator crbegin() const noexcept { return reverse_iterator(end()); }

        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator crend() const noexcept { return reverse_iterator(begin()); }

        reference at(size_type pos) { return ref(pos); }
        const_reference at(size_type pos) const { return ref(pos); }
        reference operator[](size_type pos) { return ref(pos); }
        const_reference operator[](size_type pos) const { return ref(pos); }

        reference front() { return *begin(); }
        const_reference front() const { return *begin(); }
        reference back() { return *ptr(count_ - 1); }
        const_reference back() const { return *ptr(count_ - 1);}

        value_type* data() noexcept { return ptr(0); }
        const value_type* data() const noexcept { return ptr(0); }

        constexpr size_type capacity() const { return _Size; }
        constexpr size_type max_size()  const noexcept { return _Size; }
        const size_type size() const noexcept { return count_; }
        const bool empty() const noexcept { return begin() == end(); }
        const bool full() const noexcept { return size() == max_size(); }

        void fill(const _Ty& value)
        {
            for (size_type i = 0; i < _Size && i < count_; i++)
            {
                ref(i) = value;
            }
            for (size_type i = count_; i < _Size; i++)
            {
                new (ptr(i)) _Ty(value);
            }
            count_ = _Size;
        }

        void clear()
        {
            if (!std::is_trivial<_Ty>::value)
            {
                for (size_type i = 0; i < count_; i++)
                {
                    ptr(i)->~_Ty();
                }
            }
            count_ = 0;
        }

        template<class T = _Ty>
        void push_back(const typename std::enable_if<std::is_trivial<T>::value, T>::type& value)
        {
            if (full())
            {
                return;
            }
            *ptr(count_++) = value;
        }

        template<class T = _Ty>
        void push_back(const typename std::enable_if<!std::is_trivial<T>::value, T>::type & value)
        {
            if (full())
            {
                return;
            }
            new (ptr(count_++)) _Ty(value);
        }

        template<class T = _Ty>
        void pop_back(const typename std::enable_if<std::is_trivial<T>::value>::type*  = 0)
        {
            count_--;
        }

        template<class T = _Ty>
        void pop_back(const typename std::enable_if<!std::is_trivial<T>::value>::type*  =0)
        {
            ptr(count_)->~_Ty();
            count_--;
        }

        template<class T = _Ty>
        iterator inject(const_iterator in_pos, size_type count, const typename std::enable_if<std::is_trivial<T>::value>::type* = 0)
        {
            static_assert(std::is_same<iterator, pointer>::value, "");
            if (in_pos < begin())
            {
                return end();
            }
            if (count_ + count > _Size)
            {
                return end();
            }
            iterator pos = (iterator)in_pos;
            if (pos > end())
            {
                pos = end();
            }
            iterator old_end = end();
            count_ += count;
            if (pos == old_end)
            {
                return pos;
            }
            memmove((space_type*)in_pos + count, (space_type*)in_pos, sizeof(space_type) * ((space_type*)old_end - (space_type*)in_pos));
            return pos;
        }
        template<class T = _Ty>
        iterator inject(const_iterator in_pos, size_type count, const typename std::enable_if<!std::is_trivial<T>::value>::type* = 0)
        {
            static_assert(std::is_same<iterator, pointer>::value, "");
            if (in_pos < begin())
            {
                return end();
            }
            if (count_ + count > _Size)
            {
                return end();
            }
            iterator pos = (iterator)in_pos;
            if (pos > end())
            {
                pos = end();
            }
            iterator old_end = end();
            count_ += count;
            if (pos == old_end)
            {
                return pos;
            }
            iterator new_end = end();
            iterator src = old_end;
            iterator target = new_end;
            while (src != pos && target != old_end)
            {
                new (--target) _Ty(*(--src));
            }

            while (src != pos)
            {
                *(--target) = *(--src);
            }

            src = pos;
            target = pos + count;
            while (src != target)
            {
                src++->~_Ty();
            }
            return pos;
        }

        //[first,last)
        template<class T = _Ty>
        iterator erase(const_iterator first, const_iterator last, const typename std::enable_if<std::is_trivial<T>::value>::type* = 0)
        {
            if (first >= end() || first < begin())
            {
                return end();
            }

            if (last >= end())
            {
                count_ -= distance(first, end());
                return end();
            }
            size_type island_count = distance(last, end());
            memmove((space_type*)first, (space_type*)last, island_count * sizeof(space_type));
            iterator new_end = (iterator)(first + island_count);
            count_ -= distance(new_end, end());
            return end();
        }
        template<class T = _Ty>
        iterator erase(const_iterator first, const_iterator last, const typename std::enable_if<!std::is_trivial<T>::value>::type* = 0)
        {
            if (first >= end() || first < begin())
            {
                return end();
            }

            size_type island_count = distance(last, end());
            iterator cp_first = (iterator)first;
            iterator cp_last = (iterator)last;
            for (size_type i = 0; i < island_count; i++)
            {
                *cp_first++ = *cp_last++;
            }
            iterator erase_first = cp_first;
            while (erase_first != end())
            {
                erase_first->~_Ty();
                ++erase_first;
            }
            count_ -= distance(cp_first, end());
            return end();
        }

        iterator erase(iterator pos)
        {
            return erase(pos, pos + 1);
        }

        template<class T = _Ty>
        iterator insert(iterator pos, size_type count, const typename std::enable_if <std::is_trivial<T>::value, _Ty>::type& value)
        {
            iterator new_iter = inject(pos, count);
            if (new_iter == end())
            {
                return  end();
            }
            for (size_t i = 0; i < count; i++)
            {
                *pos++ = value;
            }
            return new_iter;
        }

        template<class T = _Ty>
        iterator insert(iterator pos, size_type count, const typename std::enable_if <!std::is_trivial<T>::value, _Ty>::type& value)
        {
            iterator new_iter = inject(pos, count);
            if (new_iter == end())
            {
                return  end();
            }
            for (size_t i = 0; i < count; i++)
            {
                new (pos++) _Ty(value);
            }
            return new_iter;
        }

        iterator insert(iterator pos, const _Ty& value)
        {
            return insert(pos, 1, value);
        }

        //[first,last)
        iterator insert(iterator pos, const_iterator first, const_iterator last)
        {
            if (first >= begin() && first < begin() + _Size)
            {
                return end();
            }
            iterator new_iter = inject(pos, distance(first, last));
            if (new_iter == end())
            {
                return  end();
            }
            iterator cp_first = (iterator)first;
            if (!std::is_trivial< _Ty>::value)
            {
                while (cp_first != last)
                {
                    new (pos++) _Ty(*cp_first++);
                }
            }
            else
            {
                while (cp_first != last)
                {
                    *pos++ =  *cp_first++;
                }
            }


            return new_iter;
        }

        iterator assign(const_iterator first, const_iterator last)
        {
            clear();
            return insert(end(), first, last);
        }

        iterator assign(size_type count, const _Ty& value)
        {
            clear();
            return insert(0, value);
        }

        template< class... Args >
        iterator emplace(const_iterator pos, Args&&... args)
        {
            iterator iter = inject(pos, 1);
            if (iter == end())
            {
                return end();
            }
            new (iter) _Ty(args...);
            return iter;
        }

        template< class... Args >
        iterator emplace_back(Args&&... args)
        {
            return emplace(end(), args...);
        }

        zarray<_Ty, _Size>& operator=(const zarray<_Ty, _Size>& other)
        {
            if (this == &other)
            {
                return *this;
            }
            this->assign(other.begin(), other.end());
            return *this;
        }

    private:
        space_type data_[_Size];
        size_type count_;
    };


    template<class _Ty, size_t _Size>
    bool operator==(const zarray<_Ty, _Size>& a1, const zarray<_Ty, _Size>& a2)
    {
        return a1.data() == a2.data();
    }


}


#endif