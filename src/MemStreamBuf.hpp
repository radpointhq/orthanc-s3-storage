#ifndef MEMSTREAMBUF_HPP
#define MEMSTREAMBUF_HPP

#include <sstream>
#include <iostream>
#include <cstring>

namespace Memory {

template<typename T>
class IMemoryManager {
public:
    IMemoryManager() = default;
    virtual ~IMemoryManager() = default;

    virtual T* operator()(size_t) = 0;
    virtual void release()  = 0;
};

//good read: http://blog.httrack.com/blog/2014/04/05/a-story-of-realloc-and-laziness/
template <typename T, size_t chunkSize = 1024>
class ReallocWithBlockGrowth : public IMemoryManager<T>{
    T* _mem;
    size_t _size;
    bool _owning;
public:
    ReallocWithBlockGrowth():
        _mem(nullptr),
        _size(0),
        _owning(true)
    {
    };

    ReallocWithBlockGrowth(T* p, size_t s, bool owning = true):
        _mem(p),
        _size(s),
        _owning(owning)
    {
    };

    virtual ~ReallocWithBlockGrowth()
    {
        if (_owning)  {
            free(_mem);
        }
        _size = 0;
    }

    //no copying for now
    ReallocWithBlockGrowth(const ReallocWithBlockGrowth&) = delete;
    ReallocWithBlockGrowth& operator =(const ReallocWithBlockGrowth&) = delete;

    ReallocWithBlockGrowth(const ReallocWithBlockGrowth&&) = delete;
    ReallocWithBlockGrowth& operator =(const ReallocWithBlockGrowth&&) = delete;

    T* operator()(size_t size) override {

        if (size == 0 && _size == 0) {
            _size = chunkSize;
        } else if (size < _size) {
            return _mem;
        } else {
            _size += std::max(chunkSize, size);
        }
        _mem = static_cast<T*>(realloc(_mem, _size));
        return _mem;
    }

    void release() override {
        free(_mem);
        _size = 0;
    }

    T* get() { return _mem; }
    void set(T* p, size_t s, bool own = true) {
        _mem =  p;
        _size = s;
        _owning = own;
    }
    size_t maxSize() { return _size; }
    bool owning() { return _owning; }
};

template <typename T, size_t chunkSize = 1024>
class ReallocWithDoubleGrowth : public IMemoryManager<T>{
    T* _mem;
    size_t _size;
    bool _owning;
public:
    ReallocWithDoubleGrowth():
        _mem(nullptr),
        _size(0),
        _owning(true)
    {
    };

    ReallocWithDoubleGrowth(T* p, size_t s, bool owning = true):
        _mem(p),
        _size(s),
        _owning(owning)
    {
    };

    virtual ~ReallocWithDoubleGrowth()
    {
        if (_owning)  {
            free(_mem);
        }
        _size = 0;
    }

    //no copying for now
    ReallocWithDoubleGrowth(const ReallocWithDoubleGrowth&) = delete;
    ReallocWithDoubleGrowth& operator =(const ReallocWithDoubleGrowth&) = delete;

    ReallocWithDoubleGrowth(const ReallocWithDoubleGrowth&&) = delete;
    ReallocWithDoubleGrowth& operator =(const ReallocWithDoubleGrowth&&) = delete;

    T* operator()(size_t size) override {
        //_size = std::max(chunkSize, _size * 2);
        if (size == 0 && _size == 0) {
            _size = chunkSize;
        } else if (size < _size) {
            return _mem;
        } else {
            _size = std::max(chunkSize, std::max(size * 2, _size * 2));
        }
        _mem = static_cast<T*>(realloc(_mem, _size));
        return _mem;
    }

    void release() override {
        free(_mem);
        _size = 0;
    }

    T* get() { return _mem; }
    void set(T* p, size_t s, bool own = true) {
        _mem =  p;
        _size = s;
        _owning = own;
    }
    size_t maxSize() { return _size; }
    bool owning() { return _owning; }
};

} //namepsace

namespace Stream {

template <class Allocator = Memory::ReallocWithDoubleGrowth<std::streambuf::char_type> >
class MemStreamBufT : public std::streambuf
{
    Allocator _allocator;

public:

    MemStreamBufT()
    {
        auto beg = _allocator.get();
        auto end = _allocator.get() + _allocator.maxSize();

        setp(beg, end);
        setg(beg, beg, beg);
    }

    MemStreamBufT(char_type* p, std::streamsize s, bool owning = false):
        _allocator(p, s, owning)
    {
        auto beg = _allocator.get();
        auto end = _allocator.get() + s;//_allocator.maxSize();

        setp(beg, end);
        setg(beg, beg, beg);
    }


    char* get() { return _allocator.get(); }
    std::string str() { return std::string(_allocator.get(), size()); }

    size_t size() { return egptr() - eback(); } //content length

    bool isOwning() { return _allocator.owning(); }
    std::streamsize allocsize() { return _allocator.maxSize(); } //alloc size

    // basic_streambuf interface
protected:
    virtual pos_type seekoff(off_type __off,
                             std::ios_base::seekdir __way = std::ios_base::beg,
                             std::ios_base::openmode __which = std::ios_base::in | std::ios_base::out) override
    {
        if (__way == std::ios_base::beg) {
            return seekpos(__off, __which);
        }
        else if (__way == std::ios_base::end) {
            return seekpos(egptr() - eback() - __off, __which);
        }
        else if (__way == std::ios_base::cur) {
            if(__which & std::ios_base::in) {
                return seekpos((gptr() - eback()) + __off, __which);
            }
            else if(__which & std::ios_base::out) {
                return seekpos((pptr() - pbase()) + __off, __which);
            }
        }

        return off_type(-1);
    }

    virtual pos_type seekpos(pos_type __sp,
                             std::ios_base::openmode __which = std::ios_base::in | std::ios_base::out) override
    {
        size_t absoluteOffset = static_cast<size_t>(__sp);
        // __which == in? fail
        //check if can seek within allocated memory
        if ((pbase() + absoluteOffset) > epptr() ) {
            //if not allocate more

            if (!allocate(absoluteOffset)) {
                return pos_type(-1);
            }
        }

        //seek depending on __which
        //set p's and g's
        if (__which & std::ios_base::in) {
            //setg(_allocator.get(), _allocator.get() + absoluteOffset, pptr());
            setg(eback(), eback() + absoluteOffset, egptr());
        } else if (__which & std::ios_base::out) {
            setp(_allocator.get(), _allocator.get() + _allocator.maxSize());
            pbump(absoluteOffset);
        }
        return __sp;
    }

    virtual std::streamsize xsputn(const char_type *__s, std::streamsize __n) override {
        //check if __n chars can be written
        if (epptr() - pptr() < __n) {
            //if not allocate memory, move old memory

            if (!allocate(size() + __n)) {
                return traits_type::eof();
            }
        }

        //if so, just copy
        //std::memcpy(pptr(), __s, static_cast<size_t>(__n));
        std::copy(__s, __s + __n, pptr());

        //adjust p prts
        pbump(static_cast<int>(__n));

        //update possible reading range
        setg(eback(), gptr(), std::max(egptr(), pptr()));

        return __n;
    }

    virtual int_type overflow(int_type __c) override
    {
        if (traits_type::eq_int_type(__c,traits_type::eof()) == true) { return 0; }

        auto c = traits_type::to_char_type(__c);

        if (epptr() == pptr()) {
            if (!allocate(size() + 1)) {
                return traits_type::eof();
            }
        }

        char_type t = sputc(c);
        //update possible reading range
        setg(eback(), gptr(), std::max(egptr(), pptr()));
        return t;
    }

private:

    bool allocate(size_t newSize) {
        //size_t pOff = pptr() - pbase();
        size_t pOff = pptr() - pbase();
        size_t gOff = gptr() - eback();
        size_t egOff = egptr() - eback();

        char_type* newStart = _allocator(newSize);
        if (newStart == nullptr) {
            return false;
        }

        setp(_allocator.get(), _allocator.get() + _allocator.maxSize());
        pbump(pOff);

        setg(_allocator.get(), _allocator.get() + gOff, _allocator.get() + egOff);

        return true;
    }
};

typedef MemStreamBufT<> MemStreamBuf;

} //namespace

#endif // MEMSTREAMBUF_HPP
