#pragma once

template<typename T>
struct _aligned_allocator {
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template <class U> struct rebind { typedef _aligned_allocator<U> other; };
    _aligned_allocator() throw() {}
    _aligned_allocator(const _aligned_allocator&) throw() {}

    template <class U> _aligned_allocator(const _aligned_allocator<U>&) throw(){}

    ~_aligned_allocator() throw() {}

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type s, void const * = 0) {
        if (0 == s)
            return NULL;
        pointer temp = (pointer)_aligned_malloc(s * sizeof(T), 16);
        if (temp == NULL)
            throw std::bad_alloc();
        return temp;
    }

    void deallocate(pointer p, size_type) {
        _aligned_free(p);
    }

    size_type max_size() const throw() {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }

    void construct(pointer p, const T& val) {
        new((void *)p) T(val);
    }

    void destroy(pointer p) {
        p->~T();
    }
};

// test for allocator equality
template<class _Ty, class _Other>
inline bool operator==(const _aligned_allocator<_Ty>&, const _aligned_allocator<_Other>&) _THROW0()
{
    return true;
}
