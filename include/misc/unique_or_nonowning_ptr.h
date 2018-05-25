/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNIQUE_OR_NONOWNING_PTR_H
#define UNIQUE_OR_NONOWNING_PTR_H

#include <memory>
#include <type_traits>

/**
    This template behaves similar to std::unique_ptr (a pointer that is deleted upon destruction) but it allows to alternatively
    store a raw pointer (a pointer that is NOT deleted upon destruction). The former is the case when it is initialized or copied from
    a std::unique_ptr while the latter is done when initialized (without deleter) or copied from a raw pointer.
    The template parameter T is the type of the pointer to store (arrays are not supported). The template parameter deleter is either the
    function pointer type void (*)(T*) or a stateless functor expecting a T* as argument. Stateful functors, lambdas and std::functions
    are not supported.
*/
template <typename T, typename deleter = std::default_delete<T> >
class unique_or_nonowning_ptr {
public:
    typedef void (*deleter_func_ptr)(T*);
    typedef std::unique_ptr<T, deleter_func_ptr> internal_ptr_type;
    typedef typename internal_ptr_type::pointer pointer;
    typedef typename internal_ptr_type::element_type element_type;
    typedef typename internal_ptr_type::deleter_type deleter_type;

    static void noop_deleter(T* p) {
    }

    static void functor_deleter(T* p) {
        deleter()(p);
    }


    constexpr unique_or_nonowning_ptr() noexcept : ptr(nullptr, &noop_deleter) { };

    constexpr unique_or_nonowning_ptr(std::nullptr_t) noexcept : ptr(nullptr, &noop_deleter) { };

    unique_or_nonowning_ptr(T* p) noexcept : ptr(p, &noop_deleter) { };

    unique_or_nonowning_ptr(T* p, deleter_func_ptr d) noexcept : ptr(p, d) { };

    unique_or_nonowning_ptr(unique_or_nonowning_ptr&& u) noexcept : ptr(std::move(u.ptr)) { };

    template<typename = std::enable_if<std::is_pointer<deleter>::value>, int >
    unique_or_nonowning_ptr(std::unique_ptr<T, deleter>&& u) noexcept
     : ptr(internal_ptr_type(u.release(), u.get_deleter()) ) {
    }

    unique_or_nonowning_ptr(std::unique_ptr<T, deleter>&& u) noexcept
     : ptr(internal_ptr_type(u.release(), &functor_deleter) ) {
    }

    unique_or_nonowning_ptr& operator=(T* p) noexcept {
        ptr = internal_ptr_type(p, &noop_deleter);
        return *this;
    };

    unique_or_nonowning_ptr& operator=(unique_or_nonowning_ptr&& u) noexcept {
        ptr = std::move(u.ptr);
        return *this;
    };

    unique_or_nonowning_ptr& operator=(std::nullptr_t) noexcept {
        ptr.reset();
        return *this;
    };

    template<typename = std::enable_if<std::is_pointer<deleter>::value>, int >
    unique_or_nonowning_ptr& operator=(std::unique_ptr<T, deleter>&& u) noexcept {
        ptr = internal_ptr_type(u.release(), u.get_deleter() );
        return *this;
    }

    unique_or_nonowning_ptr& operator=(std::unique_ptr<T, deleter>&& u) noexcept {
        ptr = internal_ptr_type(u.release(), &functor_deleter);
        return *this;
    }

    unique_or_nonowning_ptr(const unique_or_nonowning_ptr&) = delete;
    unique_or_nonowning_ptr& operator=(const unique_or_nonowning_ptr&) = delete;


    typename std::add_lvalue_reference<T>::type operator*() const
    {
        return ptr.get();
    }

    pointer operator->() const noexcept
    {
        return ptr.operator->();
    }

    pointer get() const noexcept
    {
        return ptr.get();
    }

    deleter_type& get_deleter() noexcept
    {
        return ptr.get_deleter();
    }

    const deleter_type& get_deleter() const noexcept
    {
        return ptr.get_deleter();
    }

    explicit operator bool() const noexcept
    {
        return (bool) ptr;
    }

    pointer release() noexcept
    {
        return ptr.release();
    }

    void reset(pointer p = pointer()) noexcept
    {
        ptr.reset(p);
    }

    void reset(std::unique_ptr<T, deleter>&& p) noexcept
    {
        this->operator=(std::move(p));
    }

    void swap(unique_or_nonowning_ptr& u) noexcept
    {
        ptr.swap(u.ptr);
    }


private:
    internal_ptr_type ptr;
};

#endif // UNIQUE_OR_NONOWNING_PTR_H
