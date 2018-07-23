#ifndef MEMSTREAMBUF_HPP
#define MEMSTREAMBUF_HPP

#include <sstream>
#include <iostream>
#include <cstring>

namespace Stream {

  template <typename T, size_t chunkSize = 1024>
  class MemoryManager {
          char* _mem;
          size_t _size;
          bool _owning;
  public:
          MemoryManager():
          _mem(nullptr),
          _size(0)
          _owning(true)
          {
          };
          ~MemoryManager()
          {
            if (_owning)  {
              delete _mem;
            }
          }

          char* moreMemory() {
                  if (_size)

          }

          T* operator () {

          }
  };


class MemStreamBuf : public std::streambuf
{
    char_type* _buf;
    size_t _size;

    bool _owning;
public:

    MemStreamBuf():
        _buf(nullptr),
        _size(0),
        _owning(true)
    {
        auto beg = _buf;
        auto end = _buf + _size;

        setp(beg, end);
        setg(beg, beg, end);
    }

    MemStreamBuf(char_type* p, std::streamsize s, bool owning = false):
        _buf(p),
        _size(s),
        _owning(owning)
    {
        auto beg = _buf;
        auto end = _buf + _size;

        setp(beg, end);
        setg(beg, beg, end);
    }

    virtual ~MemStreamBuf() override {
        if (_owning) {
            delete[] _buf;
        }
    }

    //no copying for now
    MemStreamBuf(const MemStreamBuf&) = delete;
    MemStreamBuf& operator =(const MemStreamBuf&) = delete;

    MemStreamBuf(const MemStreamBuf&&) = delete;
    MemStreamBuf& operator =(const MemStreamBuf&&) = delete;

    char* get() { return _buf; }
    std::string str() { return std::string(_buf, size()); }

    std::streamsize allocsize() { return _size; } //alloc size
    size_t size() { return pptr() - pbase(); } //content length
    bool isOwning() { return _owning; }

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
            return seekpos(epptr() - pbase() - __off, __which);
        }
        else if (__way == std::ios_base::cur) {
            if(__which & std::ios_base::in) {
                return seekpos((gptr() - pbase()) + __off, __which);
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
        //TODO: __which == in? fail
        //check if can seek within allocated memory
        if ((pbase() + absoluteOffset) > epptr() ) {
        //if not allocate more

            if (!realloc(absoluteOffset)) {
                return pos_type(-1);
            }
        }

        //seek depending on __which
        //set p's and g's
        if (__which & std::ios_base::in) {
            setg(_buf, _buf + absoluteOffset, pptr());
        } else if (__which & std::ios_base::out) {
            setp(_buf, epptr());
            pbump(absoluteOffset);
        }
        return __sp;
    }

    virtual std::streamsize xsputn(const char_type *__s, std::streamsize __n) override
    {
        //std::cout << "xsputn(" << __n << "): " << ((__s!=nullptr)?__s[0]:'-')<< '\n';

        //TODO: check if __n chars can be written
        if (epptr() - pptr() < __n) {
            //TOOD: if not allocate memory, move old memory

            if (!realloc(size() + __n)) {
                return traits_type::eof();
            }
        }

        //TODO: if so, just copy
        std::memcpy(pptr(), __s, static_cast<size_t>(__n));

        //TODO: adjust p prts
        pbump(static_cast<int>(__n));

        return __n;
    }

    virtual int_type overflow(int_type __c) override
    {
        //std::cout << "overflow: " << static_cast<char>(__c) << '\n';
        if (traits_type::eq_int_type(__c,traits_type::eof()) == true) { return 0; }

        auto c = traits_type::to_char_type(__c);

        if (epptr() == pptr()) {
            if (!realloc(size() + 1024*1024)) {
                return traits_type::eof();
            }
        }
        /*
        std::cout << "Put: " << c << std::ios::hex << ", "
                  << "pbase: " << static_cast<void*>(pbase()) << ", "
                  << "pptr: " << static_cast<void*>(pptr()) << ", "
                  << "epptr: " << static_cast<void*>(epptr()) << ", "
                  << '\n';
                  */

        return sputc(c);
    }

    /*
    virtual int_type pbackfail(int_type __c) override
    {
        std::cout << "pbacfail: " << __c << '\n';
        return std::streambuf::pbackfail(__c);
    }
    */

private:
    bool realloc(size_t newSize) {
        size_t pOff = pptr() - pbase();
        size_t gOff = gptr() - eback();

        char_type* newStart = new char_type[newSize];
        if (newStart == nullptr) {
            return false;
        }

        if (_buf) {
            std::memcpy(newStart, _buf, size()); //copy old content
            delete[] _buf;
        }

        _buf = newStart;
        _size = newSize;

        setp(_buf, _buf + _size);
        pbump(pOff);
        setg(_buf, _buf + gOff, pptr());

        return true;
    }
};


}

#endif // MEMSTREAMBUF_HPP
